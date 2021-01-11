#include <proast/Options.hpp>
#include <gubg/mss.hpp>

namespace proast { 
    bool Options::parse(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        int arg_ix = 0;
        auto pop_arg = [&](std::string &arg)
        {
            if (arg_ix >= argc)
                return false;
            arg = argv[arg_ix++];
            return true;
        };

        MSS(pop_arg(exe_filename));

        for (std::string arg; pop_arg(arg); )
        {
            auto matches = [&](const char *short_hand, const char *long_hand)
            {
                return arg == short_hand || arg == long_hand;
            };
            if (false) {}
            else if (matches("-h", "--help")) { print_help = true; }
            else if (matches("-r", "--root"))
            {
                std::string root;
                MSS(pop_arg(root), std::cout << "Error: No root was specified" << std::endl);
                roots.push_back(root);
            }
            else { std::cout << "Error: unknown CLI argument \"" << arg << "\"" << std::endl; }
        }

        MSS_END();
    }

    void Options::stream(std::ostream &os) const
    {
        os << "[Options]{" << std::endl;
        for (const auto &root: roots)
            os << "  [Root](path:" << root << ")" << std::endl;
        os << "}" << std::endl;
    }
} 
