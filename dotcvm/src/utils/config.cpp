#include <dotcvm/utils/config.hpp>
#include <dotcvm/utils/log.hpp>
#include <dotcvm/utils/string.hpp>
#include <fstream>
#include <algorithm>

std::map<std::string, std::string> read_config_file(std::string file_path)
{
    std::map<std::string, std::string> config;
    std::ifstream file (file_path);
    if (file.is_open())
    {
        std::string line;
        while(std::getline(file, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
            if(line[0] == '#' || line.empty())
                continue;
            std::size_t delimiterPos = line.find("=");
            std::string name = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            config.insert_or_assign(name, value);
        }
    }
    else
        WARN_M("Can't open config file for reading: " << file_path);
    return config;
}

config::config(std::string file_path)
{
    m_config_map = read_config_file(file_path);
}


bool config::internal_get_bool(std::string name, bool default_value)
{
    if(m_config_map.contains(name))
    {
        return stob(m_config_map[name], default_value);
    }
    return default_value;
}
uint config::internal_get_uint(std::string name, uint default_value)
{
    if(m_config_map.contains(name))
    {
        return stoui(m_config_map[name], default_value);
    }
    return default_value;
}
int config::internal_get_int (std::string name, int default_value)
{
    return stoi(m_config_map[name], default_value);
}
ulong config::internal_get_ulong(std::string name, ulong default_value)
{
    if(m_config_map.contains(name))
    {
        return stoul(m_config_map[name], default_value);
    }
    return default_value;
}
long int config::internal_get_long(std::string name, ulong default_value)
{
    if(m_config_map.contains(name))
    {
        return stol(m_config_map[name], default_value);
    }
    return default_value;
}
std::string config::internal_get_string(std::string name, std::string default_value)
{
    if(m_config_map.contains(name))
        return m_config_map[name];
    return default_value;
}
std::vector<bool> config::internal_get_bool_array(std::string name, bool default_value)
{
    std::vector<bool> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(stob(s, default_value));
    return ret;
}
std::vector<uint> config::internal_get_uint_array(std::string name, uint default_value)
{
    std::vector<uint> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(stoui(s, default_value));
    return ret;   
}
std::vector<int> config::internal_get_int_array(std::string name, int default_value)
{
    std::vector<int> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(stoi(s, default_value));
    return ret;
}
std::vector<ulong> config::internal_get_ulong_array(std::string name, ulong default_value)
{
    std::vector<ulong> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(stoul(s, default_value));
    return ret;
}
std::vector<long int> config::internal_get_long_array(std::string name, long int default_value)
{
    std::vector<long int> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(stol(s, default_value));
    return ret;
}
std::vector<std::string> config::internal_get_string_array(std::string name, std::string default_value)
{
    std::vector<std::string> ret;
    if(m_config_map.contains(name))
        for(std::string s : string_to_string_array(m_config_map[name]))
            ret.push_back(s);
    return ret;
}

bool config_get_bool(config& c, std::string name, bool default_value) { return c.internal_get_bool(name, default_value); }
uint config_get_uint(config& c, std::string name, uint default_value) { return c.internal_get_uint(name, default_value); }
int config_get_int (config& c, std::string name, int default_value)   { return c.internal_get_int(name, default_value);  }
ulong config_get_ulong(config& c, std::string name, ulong default_value) { return c.internal_get_ulong(name, default_value); }
long int config_get_long(config& c, std::string name, ulong default_value) { return c.internal_get_long(name, default_value); }
std::string config_get_string(config& c, std::string name, std::string default_value) { return c.internal_get_string(name, default_value); }
std::vector<bool> config_get_bool_array(config& c, std::string name, bool default_value) { return c.internal_get_bool_array(name, default_value); }
std::vector<uint> config_get_uint_array(config& c, std::string name, uint default_value) { return c.internal_get_uint_array(name, default_value); }
std::vector<int> config_get_int_array(config& c, std::string name, int default_value) { return c.internal_get_int_array(name, default_value); }
std::vector<ulong> config_get_ulong_array(config& c, std::string name, ulong default_value) { return c.internal_get_ulong_array(name, default_value); }
std::vector<long int> config_get_long_array(config& c, std::string name, long int default_value) { return c.internal_get_long_array(name, default_value); }
std::vector<std::string> config_get_string_array(config& c, std::string name, std::string default_value) { return c.internal_get_string_array(name, default_value); }