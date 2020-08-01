#ifndef DC_MODULES_H
#define DC_MODULES_H

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <dotcvm/dotcvm.hpp>

#define MODULES_DIR "modules/"

enum module_clock           { NEVER, FIRST, CPU, NORMAL, LAST, FIRST_CPU, FIRST_NORMAL, FIRST_LAST, CPU_LAST, NORMAL_LAST, ALL };
enum module_connection_mode { AGREEMENT, ACCEPT_ALL, DECLINE_ALL };

struct module
{
    uint                                uid;
    std::map<std::string, std::string>  config              ;
    void*                               lib_handle          = nullptr;
    uint                                id                  ;
    std::string                         name                = "<unknown>";
    std::string                         lib_file            = "device";
    module_clock                        clock_mode          = module_clock::NORMAL;
    module_connection_mode              connection_mode     = module_connection_mode::AGREEMENT;
    std::vector<uint>                   require_list        ;
    std::vector<uint>                   connect_list        ;
    std::vector<uint>                   actual_connections  ;
    device_ptr                          p_device            ;

    void        (*fp_report)            (uint additional_data0, uint additional_data1);
    device_ptr  (*fp_create_device)     ();
    void        (*fp_pre_clock)         ();
    void        (*fp_clock)             ();
    void        (*fp_post_clock)        ();
    void        (*fp_destroy_device)    (device_ptr device);

    void        report(uint additional_data0, uint additional_data1);
    void        create_device   ();
    void        pre_clock       ();
    void        clock           ();
    void        post_clock      ();
    void        destroy_device  ();
};

struct module_report
{
    module*     target_module;
    uint        additional_data0;
    uint        additional_data1;
};

extern void load_modules();
extern void clock_modules();
extern void unload_modules();

#endif /* DC_MODULES_H */