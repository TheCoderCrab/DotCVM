#include <dotcvm/utils/utils.hpp>
#include <filesystem>

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif /* __linux__ */

namespace utils
{
    std::string get_workdir()
    {
#ifdef DEBUG
        return "./";
#else
#ifdef __linux__
        std::string home;
        if(getenv("HOME") == nullptr)
            home = std::string(getpwuid(getuid())->pw_dir);
        else
            home = std::string(getenv("HOME"));
        if(!std::filesystem::exists(home + "/.local/share/dotcvm"))
            std::filesystem::create_directory(home + "/.local/share/dotcvm");
        return home + "/.local/share/dotcvm/";
#endif
#endif /* DEBUG */
    }
}