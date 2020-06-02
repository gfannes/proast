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
            else if (arg == "-r" || arg == "--root") { roots.push_back(std::filesystem::canonical(pop_arg())); }
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
        oss << "    -h,--help        Print this help" << std::endl;
        oss << "    -v,--version     Print version information" << std::endl;
        oss << "    -r,--root <PATH> Use <PATH> as root" << std::endl;

        return oss.str();
    }

    void Options::stream(std::ostream &os) const
    {
        os << "[Options]{" << std::endl;
        os << "  [print_help](bool:" << print_help << ")" << std::endl;
        os << "  [print_version](bool:" << print_version << ")" << std::endl;
        for (const auto &root: roots)
            os << "  [root](path:" << root << ")" << std::endl;
        os << "}" << std::endl;
    }
} 
