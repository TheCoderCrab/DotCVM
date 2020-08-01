#ifndef DC_STRING_H
#define DC_STRING_H

#include <string>
#include <vector>

extern std::string trim(std::string& str);
extern std::string remove_any(std::string& str, char c);
extern std::vector<std::string> split(std::string& str, char r);
extern std::vector<uint> string_to_uint_array(std::string& str);

#endif /* DC_STRING_H */