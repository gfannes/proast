#include <proast/Options.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <sstream>

namespace proast { 
    bool Options::parse(const int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        std::size_t ix = 0;
        auto pop_arg = [&]() -> std::string
        {
            const std::string arg = (ix < argc ? argv[ix] : "");
            ++ix;
            return arg;
        };

        exe_name = pop_arg();

        for (; ix < argc;)
        {
            const std::string arg = pop_arg();

            if (false) {}
            else if (arg == "-h" || arg == "--help") { print_help = true; }
            else if (arg == "-v" || arg == "--version") { print_version = true; }
            else
            {
                MSS(false, log::stream() << "Error: unknown command-line argument \"" << arg << "\"" << std::endl);
            }
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        std::ostringstream oss;

        oss << exe_name << std::endl;
        oss << "    -h    Print this help" << std::endl;
        oss << "    -v    Print version information" << std::endl;

        return oss.str();
    }
} 
