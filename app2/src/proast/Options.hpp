#ifndef HEADER_proast_Options_hpp_ALREADY_INCLUDED
#define HEADER_proast_Options_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>

namespace proast { 
    class Options
    {
    public:
        std::string exe_filename;
        bool print_help = false;
        std::vector<std::string> roots;

        bool parse(int argc, const char **argv);

        void stream(std::ostream &os) const;
    private:
    };
} 

#endif
