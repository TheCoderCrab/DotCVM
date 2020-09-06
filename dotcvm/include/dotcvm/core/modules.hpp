#ifndef DC_MODULES_H
#define DC_MODULES_H

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <dotcvm/dotcvm.hpp>

enum module_clock           { NEVER, FIRST, CPU, NORMAL, LAST, FIRST_CPU, FIRST_NORMAL, FIRST_LAST, CPU_LAST, NORMAL_LAST, ALL };
enum module_connection_mode { AGREEMENT, ACCEPT_ALL, DECLINE_ALL };

enum module_current_job { CREATE, INIT, PRE_CLOCK, CLOCK, POST_CLOCK, DESTROY };

struct module
{
    std::string                         module_folder       ;
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

    void        (*fp_report         )    (uint additional_data0, uint additional_data1);
    device_ptr  (*fp_create_device  )    (dotcvm_data d                               );
    void        (*fp_init           )    (                                            );
    void        (*fp_pre_clock      )    (uint cycles                                 );
    void        (*fp_clock          )    (uint cycles                                 );
    void        (*fp_post_clock     )    (uint cycles                                 );
    void        (*fp_destroy_device )    (device_ptr device                           );

    void        report          (uint additional_data0, uint additional_data1);
    void        create_device   (                                            );
    void        init            (                                            );
    void        pre_clock       (uint cycles                                 );
    void        clock           (uint cycles                                 );
    void        post_clock      (uint cycles                                 );
    void        destroy_device  (                                            );
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
extern uint module_count();
extern std::string get_module_error();

#endif /* DC_MODULES_H */