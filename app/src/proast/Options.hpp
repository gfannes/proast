#ifndef HEADER_proast_Options_hpp_ALREADY_INCLUDED
#define HEADER_proast_Options_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { 

    //POD-like class that provides access to the parsed and interpreted command-line arguments

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        bool print_version = false;

        bool parse(const int argc, const char **argv);

        std::string help() const;

    private:
    };

} 

#endif
