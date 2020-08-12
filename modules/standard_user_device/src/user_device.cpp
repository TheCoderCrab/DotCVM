#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/log.hpp>
#include <modules/io_bus.hpp>
#include <modules/user_device/user_device.hpp>
#include <modules/interrupt_bus.hpp>
#include <modules/user_device/window.hpp>
#include <string>
#include <vector>

#ifdef DEBUG
static std::string s_win_title = "DotCVM_DEBUG";
#else
static std::string s_win_title = "DotCVM";
#endif

dotcvm_data s_dc;

static uint32_t s_screen_buffer[720 * 480];

static bool s_use_io_bus = true;
static io_bus*  sp_io_bus = nullptr;

static bool s_use_interrupt_bus = true;
static interrupt_bus* sp_interrupt_bus = nullptr;

static uint s_last_key = 0;
static uint s_last_button = 0;
static uint s_mouse_x = 0;
static uint s_mouse_y = 0;

static uint s_screen_x = 0;
static uint s_screen_y = 0;
static uint s_screen_rgba = 0;

static std::vector<interrupt_data> s_interrupt_queue;


void interrupt(interrupt_data d)
{
    s_interrupt_queue.push_back(d);
}

__export void module_report(uint additional_data0, uint additional_data1)
{
    if(additional_data0 == DC_CONNECTION_INAGREEMENT || additional_data0 == DC_CONNECTION_INEXISTANT)
    {
        if (additional_data1 == DC_STD_IOB)
        {
            WARN_M("USER_DEVICE: Reported that io bus doesn't exist");
            s_use_io_bus = false;
        }
        else if(additional_data1 == DC_STD_ITB)
        {
            WARN_M("USER_DEVICE: Reported that interrupt bus doesn't exist");
            s_use_interrupt_bus = false;
        }
    }
}

__export device_ptr module_create_device(dotcvm_data d)
{
    DEBUG_M("Creating user device");
    s_dc = d;
    main_win = setup_main_window(s_win_title, 720, 480);
    return nullptr;
}

__export void module_init()
{
    if(s_use_io_bus)
        sp_io_bus = (io_bus*) s_dc.fp_get_device(DC_STD_IOB);;
    if(s_use_interrupt_bus)
        sp_interrupt_bus = (interrupt_bus*) s_dc.fp_get_device(DC_STD_ITB);
    if(sp_io_bus == nullptr)
        DEBUG_M("io");
    if(sp_interrupt_bus == nullptr)
        DEBUG_M("interrupt");
    
}

__export void module_pre_clock(uint cycles)
{
    update_main_window();
    if(sp_interrupt_bus->mode == interrupt_mode::NONE && s_interrupt_queue.size() > 0)
    {
        if(s_interrupt_queue[0].interrupt_code == EVENT_MOUSE_MOTION)
        {
            s_mouse_x = s_interrupt_queue[0].data0;
            s_mouse_y = s_interrupt_queue[0].data1;
        }
        if(s_interrupt_queue[0].interrupt_code == EVENT_MOUSE_PRESS
        || s_interrupt_queue[0].interrupt_code == EVENT_MOUSE_RELEASE)
        {
            s_last_button = s_interrupt_queue[0].data0;
        }
        if(s_interrupt_queue[0].interrupt_code == EVENT_KEYBOARD_PRESS
        || s_interrupt_queue[0].interrupt_code == EVENT_KEYBOARD_RELEASE)
        {
            s_last_key = s_interrupt_queue[0].data0;
        }
        sp_interrupt_bus->interrupt_code = s_interrupt_queue[0].interrupt_code;
        sp_interrupt_bus->mode = s_interrupt_queue[0].mode;
        s_interrupt_queue.erase(s_interrupt_queue.begin());
    }
    if(s_use_io_bus)
    {
        if(sp_io_bus->address == DC_STD_KEB)
            sp_io_bus->value = s_last_key;
        if(sp_io_bus->address == DC_STD_MSP)
            sp_io_bus->value = ((s_mouse_x << 16) & 0xF0) | (s_mouse_y & 0x0F);
        if(sp_io_bus->address == DC_STD_MSB)
            sp_io_bus->value == s_last_button;
    }
}

__export void module_post_clock(uint cycles)
{
    if(s_use_io_bus)
    {
        if(sp_io_bus->address == DC_STD_SCX)
            s_screen_x = sp_io_bus->value;
        if(sp_io_bus->address == DC_STD_SCY)
            s_screen_y = sp_io_bus->value;
        if(sp_io_bus->address == DC_STD_SCC)
            s_screen_rgba = sp_io_bus->value;
        if(sp_io_bus->address == DC_STD_SCB)
            s_screen_buffer[s_screen_x + s_screen_y * 720] = s_screen_rgba;
        if(sp_io_bus->address == DC_STD_SCD)
            for(uint x = 0; x < 720; x++)
                for(uint y = 0; y < 480; y++)
                    draw_pixel(main_win, x, y, s_screen_buffer[x + y * 720]);
    }
}

__export void module_destroy_device(device_ptr p)
{
    close_window(main_win);
    close_display();
}