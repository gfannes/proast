#ifndef HEADER_proast_Options_hpp_ALREADY_INCLUDED
#define HEADER_proast_Options_hpp_ALREADY_INCLUDED

#include <string>
#include <filesystem>
#include <vector>
#include <ostream>

namespace proast { 

    //POD-like class that provides access to the parsed and interpreted command-line arguments

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        bool print_version = false;

        std::vector<std::filesystem::path> roots;

        bool parse(const int argc, const char **argv);

        std::string help() const;

        void stream(std::ostream &os) const;

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Options &options)
    {
        options.stream(os);
        return os;
    }

} 

#endif
