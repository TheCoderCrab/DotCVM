#ifndef DC_DOTCVM_H
#define DC_DOTCVM_H

#include <cstdint>
#include <string>
#include <dotcvm/utils/config.hpp>

typedef void* device_ptr;

struct dotcvm_data
{
    void        (*fp_shutdown)(uint exit_code, std::string exit_message);
    device_ptr  (*fp_get_device)(uint requested_device);
    config      (*fp_read_module_config)(std::string config_file);


    bool (*fp_config_get_bool)(config& c, std::string name, bool default_value);
    uint (*fp_config_get_uint)(config& c, std::string name, uint default_value);
    int (*fp_config_get_int)(config& c, std::string name, int default_value);
    ulong (*fp_config_get_ulong)(config& c, std::string name, ulong default_value);
    long int (*fp_config_get_long)(config& c, std::string name, ulong default_value);
    std::string (*fp_config_get_string)(config& c, std::string name, std::string default_value);
    std::vector<bool> (*fp_config_get_bool_array)(config& c, std::string name, bool default_value);
    std::vector<uint> (*fp_config_get_uint_array)(config& c, std::string name, uint default_value);
    std::vector<int> (*fp_config_get_int_array)(config& c, std::string name, int default_value);
    std::vector<ulong> (*fp_config_get_ulong_array)(config& c, std::string name, ulong default_value);
    std::vector<long int> (*fp_config_get_long_array)(config& c, std::string name, long int default_value);
    std::vector<std::string> (*fp_config_get_string_array)(config& c, std::string name, std::string default_value);

};

#define DC_CONNECTION_INAGREEMENT       0x10
#define DC_CONNECTION_INEXISTANT        0x20

#define __export extern "C"

#endif /* DC_DOTCVM_H */