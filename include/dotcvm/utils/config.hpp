#ifndef DC_CONFIG_H
#define DC_CONFIG_H

#include <map>
#include <string>

std::map<std::string, std::string> read_config_file(std::string file_path);

#endif /* DC_CONFIG_H */