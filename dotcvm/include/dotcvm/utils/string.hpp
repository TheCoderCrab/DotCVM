#ifndef DC_STRING_H
#define DC_STRING_H

#include <string>
#include <vector>

extern std::string trim(std::string& str);
extern std::string remove_any(std::string& str, char c);
extern std::vector<std::string> split(std::string& str, char r);
extern std::vector<uint> string_to_uint_array(std::string& str);
extern std::vector<std::string> string_to_string_array(std::string& str);

extern bool stob(std::string str, bool default_value = false);
extern uint stoui(std::string str, uint default_value = 0);
extern int  stoi(std::string str, int default_value = 0);
extern ulong stoul(std::string str, ulong default_value = 0L);
extern long int stol(std::string str, long int default_value = 0L);

#endif /* DC_STRING_H */