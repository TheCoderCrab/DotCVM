#include <dotcvm/core/modules.hpp>
#include <dotcvm/utils/log.hpp>
#include <dotcvm/utils/config.hpp>
#include <dotcvm/utils/string.hpp>
#include <dotcvm/main.hpp>
#include <vector>
#include <filesystem>
#include <dlfcn.h>
#include <sstream>

device_ptr get_device(uint);

static std::vector<module> s_modules;


static std::vector<module*> s_modules_first;
static std::vector<module*> s_modules_cpus;
static std::vector<module*> s_modules_normal;
static std::vector<module*> s_modules_last;

static module s_null_module;

static module* s_clocking_module; // The module that is being update
static module_current_job s_module_current_job;

static dotcvm_data s_dotcvm_data = 
{
    .fp_shutdown = shutdown,
    .fp_get_device = get_device,
    .fp_read_module_config = read_module_config,
    .fp_config_get_bool = config_get_bool,
    .fp_config_get_uint = config_get_uint,
    .fp_config_get_int = config_get_int,
    .fp_config_get_ulong = config_get_ulong,
    .fp_config_get_long = config_get_long,
    .fp_config_get_string = config_get_string,
    .fp_config_get_bool_array = config_get_bool_array,
    .fp_config_get_uint_array = config_get_uint_array,
    .fp_config_get_int_array = config_get_int_array,
    .fp_config_get_ulong_array = config_get_ulong_array,
    .fp_config_get_long_array = config_get_long_array,
    .fp_config_get_string_array = config_get_string_array
};

static void remove_modules(std::vector<module*>& modules_to_remove)
{
    for(uint i = 0; i < s_modules.size(); i++)
        for(uint j = 0; j < modules_to_remove.size(); j++)
            if(s_modules[i].id == modules_to_remove[j]->id)
            {
                WARN_M("Removing module named: " << s_modules[i].name << " with id: " << s_modules[i].id);
                if(s_modules[i].lib_handle != nullptr)
                    dlclose(s_modules[i].lib_handle);
                s_modules.erase(s_modules.begin() + i);
                i--;
            }
    modules_to_remove.clear();
}

static bool module_exist(uint id)
{
    DEBUG_M("Looking for module with id: " << id);
    for(module m : s_modules)
    {
        DEBUG_M("Checking: " << m.id);
        if(m.id == id)
        {
            DEBUG_M("Found");
            return true;
        }
    }
    DEBUG_M("Not found");
    return false;
}

static module& get_module(uint id)
{
    for(module& m : s_modules)
        if(m.id == id)
            return m;
    WARN_M("Module with id: " << id << "not found returning null module");
    return s_null_module;
}

// This checks wether module with id0 wants to connect to module with id1
static bool module_connect_to(uint id0, uint id1)
{
    module& connect_to_module = get_module(id0);
    if(connect_to_module.connection_mode == module_connection_mode::ACCEPT_ALL)
        return true;
    if(connect_to_module.connection_mode == module_connection_mode::DECLINE_ALL)
        return false;
    for(uint connection : connect_to_module.connect_list)
        if(connection == id1)
            return true;
    return false;
}

void load_modules()
{
    std::vector<module_report> modules_reports;
    if(!std::filesystem::exists(MODULES_DIR))
        std::filesystem::create_directory(MODULES_DIR);
    uint uid_count = 0;
    for(auto& entry : std::filesystem::directory_iterator(MODULES_DIR))
        if(entry.is_directory() && std::filesystem::exists(entry.path().string().append("/device.dpf")))
        {
            s_modules.push_back(module{.module_folder = entry.path().string(), .uid = uid_count, .config = read_config_file(entry.path().string().append("/device.dpf"))});
            uid_count++;
            LOG_M("Found module at: " << entry.path().string().append("/device.dpf"));
        }
    for(int i = 0; i < s_modules.size(); i++)
    {
        module& m = s_modules[i];
        if(m.config.contains("name"))
            m.name = m.config["name"];
        if(m.config.contains("lib_file"))
            m.lib_file = m.config["lib_file"];
        if(m.config.contains("clock_mode"))
            m.clock_mode = (module_clock) (std::stoi(m.config["clock_mode"]));
        if(m.config.contains("connection_mode"))
            m.connection_mode = (module_connection_mode) (std::stoi(m.config["connection_mode"]));
        if(m.config.contains("require"))
            m.require_list = string_to_uint_array(m.config["require"]);
        if(m.config.contains("connect_to"))
            m.connect_list = string_to_uint_array(m.config["connect_to"]);
        if(m.config.contains("id"))
        {
            m.id = std::stoul(m.config["id"], nullptr, 0);
            DEBUG_M("id: " << m.name << " is " << m.id);
        }
        else
        {
            WARN_M("module: " << m.name << " is missing 'id' property, ignoring it");
            s_modules.erase(s_modules.begin() + i);
            i--;   
        }
    }
    {
        std::vector<module*> modules_to_remove;
        for(module& m : s_modules)
            for(module& m1 : s_modules)
                if(m.id == m1.id && m.uid != m1.uid)
                {
                    modules_to_remove.push_back(&m);
                    modules_to_remove.push_back(&m1);
                    WARN_M("Found duplicate of module with id: " << m.id << ", on modules named: " << m.name << " and " << m1.name);
                    DEBUG_M(s_modules.size());
                }
        remove_modules(modules_to_remove);

        for(module& m : s_modules)
        {
            std::stringstream file_path_stream;
            file_path_stream << m.module_folder << "/" << m.lib_file;
#ifdef __linux__
            file_path_stream << ".linux.so"; 
#endif
#ifdef __WIN32
            file_path_stream << ".win.so";
#endif
#ifdef __APPLE__
            file_path_stream << ".apple.so";
#endif
            DEBUG_M(m.name << ":" << m.id << ", lib file = " << file_path_stream.str().c_str());
            m.lib_handle = dlopen(file_path_stream.str().c_str(), RTLD_LAZY);
            if(m.lib_handle != nullptr)
            {
                dlerror(); // Make sure that there is no error waiting to be displayed
                m.fp_create_device = (device_ptr(*)(dotcvm_data))(dlsym(m.lib_handle, "module_create_device"));
                if(dlerror() != nullptr)
                {
                    WARN_M("Cannot load module named: " << m.name << " with id: " << m.id << ". Can't load module_create_device function.");
                    modules_to_remove.push_back(&m);   
                }
                m.fp_destroy_device = (void(*)(device_ptr))(dlsym(m.lib_handle, "module_destroy_device"));
                if(dlerror() != nullptr)
                {
                    WARN_M("Cannot load module named: " << m.name << " with id: " << m.id << ". Can't load module_destroy_device function.");
                    modules_to_remove.push_back(&m);   
                }
                m.fp_report     = (void(*)(uint,uint)  )           (dlsym(m.lib_handle, "module_report"    ));
                m.fp_init       = (void(*)()           )           (dlsym(m.lib_handle, "module_init"      ));
                m.fp_pre_clock  = (void(*)(uint cycles))           (dlsym(m.lib_handle, "module_pre_clock" ));
                m.fp_clock      = (void(*)(uint cycles))           (dlsym(m.lib_handle, "module_clock"     ));
                m.fp_post_clock = (void(*)(uint cycles))           (dlsym(m.lib_handle, "module_post_clock"));
            }
            else
            {
                WARN_M("Cannot load module named: " << m.name << " with id: " << m.id << ". Cannot load the whole library file");
                ERR_M(dlerror());
                modules_to_remove.push_back(&m);
            }
        }
        remove_modules(modules_to_remove);
        for(module& m : s_modules)
            for(uint dep : m.require_list)
                if(!module_exist(dep))
                {
                    modules_to_remove.push_back(&m);
                    WARN_M("Missing module dependecie with id: " << dep << " for the module named: " << m.name << " with id: " << m.id);
                }
        remove_modules(modules_to_remove);
        for(module& m : s_modules)
            for(uint connection : m.connect_list)
                if(module_exist(connection))
                {
                    if(module_connect_to(connection, m.id))
                        m.actual_connections.push_back(connection);
                    else
                    {
                        WARN_M("Cannot establish connection between device with id: " << m.name << ':' << m.id << " and device with id: " << connection << ", the second device doesn't agree with the connection.");
                        modules_reports.push_back(module_report{&m, DC_CONNECTION_INAGREEMENT, connection});
                    }  
                }
                else
                {
                    WARN_M("Cannot establish connection between device with id: " << m.name << ':' << m.id << " and device with id: " << connection << ", the second device doesn't exist.");
                    modules_reports.push_back(module_report{&m, DC_CONNECTION_INEXISTANT, connection});
                }
        for(module& m : s_modules)
        {
            using mc = module_clock;
            mc& c = m.clock_mode;
            if(c == mc::FIRST || c == mc::FIRST_CPU || c == mc::FIRST_NORMAL || c == FIRST_LAST)
                s_modules_first.push_back(&m);
            if(c == mc::CPU || c == mc::FIRST_CPU || c == mc::CPU_LAST)
                s_modules_cpus.push_back(&m);
            if(c == mc::NORMAL || c == mc::FIRST_NORMAL || c == mc::NORMAL_LAST)
                s_modules_normal.push_back(&m);
            if(c == mc::LAST || c == mc::FIRST_LAST || c == mc::CPU_LAST || c == mc::NORMAL_LAST)
                s_modules_last.push_back(&m);
        }
        for(module_report& r : modules_reports)
        {
            s_clocking_module = r.target_module;
            r.target_module->report(r.additional_data0, r.additional_data1);
        }
        s_module_current_job = module_current_job::CREATE;
        for(module& m : s_modules)
        {
            s_clocking_module = &m;
            m.create_device();
        }
        s_module_current_job = module_current_job::INIT;
        for(module& m : s_modules)
        {
            s_clocking_module = &m;
            m.init();
        }
    }
}

void clock_modules()
{
    static uint cycles = 0;
    s_module_current_job = module_current_job::PRE_CLOCK;
    for(module* m : s_modules_first)
    {
        s_clocking_module = m;
        m->pre_clock(cycles);
    }
    s_module_current_job = module_current_job::CLOCK;
    for(module* m : s_modules_normal)
    {
        s_clocking_module = m;
        m->clock(cycles);
    }
    for(module* m : s_modules_cpus)
    {
        s_clocking_module = m;
        m->clock(cycles);
    }
    s_module_current_job = module_current_job::POST_CLOCK;
    for(module* m : s_modules_last)
    {
        s_clocking_module = m;
        m->post_clock(cycles);
    }
    cycles++;
}

device_ptr get_device(uint requested_device)
{
    DEBUG_M(s_clocking_module->name << ':' << s_clocking_module->id << " is requesting: " << requested_device);
    for(uint connection : s_clocking_module->actual_connections)
        if(connection == requested_device)
        {
            DEBUG_M(s_clocking_module->name << ": Device with id: " << requested_device << " was found under name: " << get_module(connection).name);
            return get_module(connection).p_device;
        }
    WARN_M("The module " << s_clocking_module->name << ':' << s_clocking_module->id << " requested a device which it doesn't have connection to or simply doesn't exist, request device id: " << requested_device);
    return nullptr;
}

void unload_modules()
{
    s_module_current_job = module_current_job::DESTROY;
    for(module& m : s_modules)
    {
        s_clocking_module = &m;
        m.destroy_device();
        dlclose(m.lib_handle);
    }
}

uint module_count()
{
    return s_modules.size();
}

config read_module_config(std::string config_file)
{
    return config(s_clocking_module->module_folder.append("/").append(config_file));
}

std::string get_module_error()
{
    std::stringstream ss;
    ss << s_clocking_module->name << ':' << s_clocking_module->id << " while on step: ";
    switch (s_module_current_job)
    {
    case module_current_job::CREATE:
        ss << "CREATE";
        break;
    case module_current_job::INIT:
        ss << "INIT";
        break;
    case module_current_job::PRE_CLOCK:
        ss << "PRE_CLOCK";
        break;
    case module_current_job::CLOCK:
        ss << "CLOCK";
        break;
    case module_current_job::POST_CLOCK:
        ss << "POST_CLOCK";
        break;
    case module_current_job::DESTROY:
        ss << "DESTROY";
        break;
    }
    return ss.str();
}

void module::report(uint additional_data0, uint additional_data1)
{
    if(fp_report != nullptr)
        fp_report(additional_data0, additional_data1);
}
void module::create_device()
{
    if(fp_create_device != nullptr)
        p_device = fp_create_device(s_dotcvm_data);
}
void module::init()
{
    if(fp_init != nullptr)
        fp_init();
}
void module::pre_clock(uint cycles)
{
    if(fp_pre_clock != nullptr)
        fp_pre_clock(cycles);
}
void module::clock(uint cycles)
{
    if(fp_clock != nullptr)
        fp_clock(cycles);
}
void module::post_clock(uint cycles)
{
    if(fp_post_clock != nullptr)
        fp_post_clock(cycles);
}
void module::destroy_device()
{
    if(fp_destroy_device != nullptr)
        fp_destroy_device(p_device);
}
