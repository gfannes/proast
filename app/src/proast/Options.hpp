#ifndef HEADER_proast_Options_hpp_ALREADY_INCLUDED
#define HEADER_proast_Options_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <filesystem>

namespace proast { 
    class Options
    {
    public:
        std::string exe_filename;
        bool print_help = false;
        unsigned int verbose = 0;
        std::vector<std::string> roots;
        std::filesystem::path home_dir;
        std::string command;

        bool parse(int argc, const char **argv);

        void stream(std::ostream &os) const;

        std::string help() const;
    private:
    };
} 

#endif
