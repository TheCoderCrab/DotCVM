#include <dotcvm/utils/string.hpp>
#include <dotcvm/utils/log.hpp>
#include <string>
#include <sstream>
#include <cctype>

std::string trim(std::string& str)
{
    for(char c = str[0]; std::isspace(c); c = str[0])
        str.erase(str.begin());
    for(char c = str[str.size() - 1]; std::isspace(c); c = str[str.size() - 1])
        str.erase(str.end() - 1);
    return str;
}

std::string remove_any(std::string& str, char c)
{
    for(uint i = 0; i < str.size(); i++)
        if(str[i] == c)
        {
            str.erase(str.begin()+i);
            i--;
        }
    return str;
}

std::vector<std::string> split(std::string& str, char r)
{
    std::vector<std::string> splited;
    std::stringstream s;
    for(char c : str)
        if(c == r)
        {
            splited.push_back(s.str());
            s = std::stringstream();
        }
        else
            s << c;
    splited.push_back(s.str()); // The last element
    return splited;
}

template<typename T> std::string vector_to_string(std::vector<T>& v)
{
    std::stringstream ss;
    ss << '[';
    for(uint i = 0; i < v.size() - 1; i++)
    {
        ss << v[i];
        ss << ", ";
    }
    ss << v[v.size() - 1];
    ss << ']';
    return ss.str();
}

std::vector<uint> string_to_uint_array(std::string& str)
{
    remove_any(str, ' ');
    remove_any(str, '\t');
    if(str[0] != '[' || str[str.size() - 1] != ']')
    {
        warn("The given string doesn't represent an array");
        return std::vector<uint>();
    }
    str.erase(str.begin());
    str.erase(str.end() - 1);
    std::vector<std::string> string_splited = split(str, ',');
    debug("splited: " << vector_to_string<std::string>(string_splited));
    std::vector<uint> arr;
    for(std::string s : string_splited)
    {
        try
        {
            arr.push_back((uint) std::stoi(s, nullptr, 0));
        }
        catch(const std::invalid_argument& e)
        {
            warn("The string:" << s << " doesn't represent an integer");
            return std::vector<uint>();
        }
        catch(const std::out_of_range& e)
        {
            warn("The given integer is too large to be processed");
            return std::vector<uint>();
        }
    }
    return arr;
}