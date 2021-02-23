#include <proast/Options.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/mss.hpp>
#include <iostream>
#include <cstdlib>

namespace proast { 
    bool Options::parse(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        if (auto home = std::getenv("HOME"))
        {
            home_dir = home;
            home_dir += "/.config/proast";
        }

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
            else if (matches("-V", "--verbose"))
            {
                std::string level;
                MSS(pop_arg(level), std::cout << "Error: No verbose level was specified" << std::endl);
                verbose_level = std::stoul(level);
            }
            else if (matches("-L", "--home"))
            {
                std::string dir;
                MSS(pop_arg(dir), std::cout << "Error: No home directory was specified" << std::endl);
                home_dir = dir;
            }
            else if (matches("-c", "--command"))
            {
                MSS(pop_arg(command), std::cout << "Error: No command was specified" << std::endl);
            }
            else { std::cout << "Error: unknown CLI argument \"" << arg << "\"" << std::endl; }
        }

        MSS_END();
    }

    void Options::stream(std::ostream &os) const
    {
        gubg::naft::Document doc{os};
        {
            auto options_node = doc.node("Options");
            options_node.attr("verbose_level", verbose_level);
            {
                auto home_node = options_node.node("Home");
                home_node.attr("path", home_dir);
                if (!command.empty())
                    home_node.attr("command", command);
                for (const auto &root: roots)
                {
                    auto root_node = home_node.node("Root");
                    root_node.attr("path", root);
                }
            }
        }
    }

    std::string Options::help() const
    {
        return R"EOF(Help for proast, the PROject ASsisTant.
-h    --help            Print this help
-r    --root    FOLDER  Add FOLDER as root tree, multiple roots are supported
-V    --verbose LEVEL   Set verbosity LEVEL
-L    --home    FOLDER  Set home FOLDER, default is $HOME/.config/proast
-c    --command STRING  Run command in each root and exit
)EOF";
    }
} 
