#ifndef DC_CONFIG_H
#define DC_CONFIG_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#ifdef DOTCVM
std::map<std::string, std::string> read_config_file(std::string file_path);
#endif

class config
{
private:
    std::map<std::string, std::string> m_config_map;

public:
    config(std::string file_path);
public:
#ifdef DOTCVM
    bool internal_get_bool(std::string name, bool default_value = false);
    uint internal_get_uint(std::string name, uint default_value = 0);
    int internal_get_int (std::string name, int default_value = 0);
    ulong internal_get_ulong(std::string name, ulong default_value = 0L);
    long int internal_get_long(std::string name, ulong default_value = 0L);
    std::string internal_get_string(std::string name, std::string default_value = "");
    std::vector<bool> internal_get_bool_array(std::string name, bool default_value = false);
    std::vector<uint> internal_get_uint_array(std::string name, uint default_value = 0);
    std::vector<int> internal_get_int_array(std::string name, int default_value = 0);
    std::vector<ulong> internal_get_ulong_array(std::string name, ulong default_value = 0L);
    std::vector<long int> internal_get_long_array(std::string name, long int default_value = 0L);
    std::vector<std::string> internal_get_string_array(std::string name, std::string default_value = "");
#endif
};

#ifdef DOTCVM
extern bool config_get_bool(config& c, std::string name, bool default_value = false);
extern uint config_get_uint(config& c, std::string name, uint default_value = 0);
extern int config_get_int (config& c, std::string name, int default_value = 0);
extern ulong config_get_ulong(config& c, std::string name, ulong default_value = 0L);
extern long int config_get_long(config& c, std::string name, ulong default_value = 0L);
extern std::string config_get_string(config& c, std::string name, std::string default_value = "");
extern std::vector<bool> config_get_bool_array(config& c, std::string name, bool default_value = false);
extern std::vector<uint> config_get_uint_array(config& c, std::string name, uint default_value = 0);
extern std::vector<int> config_get_int_array(config& c, std::string name, int default_value = 0);
extern std::vector<ulong> config_get_ulong_array(config& c, std::string name, ulong default_value = 0L);
extern std::vector<long int> config_get_long_array(config& c, std::string name, long int default_value = 0L);
extern std::vector<std::string> config_get_string_array(config& c, std::string name, std::string default_value = "");
#endif

extern config read_module_config(std::string config_file);

#endif /* DC_CONFIG_H */