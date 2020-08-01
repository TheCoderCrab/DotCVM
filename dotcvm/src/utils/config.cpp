#include <dotcvm/utils/config.hpp>
#include <dotcvm/utils/log.hpp>
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
        warn("Can't open config file for reading: " << file_path);
    return config;
}