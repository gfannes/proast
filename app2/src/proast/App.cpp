#include <proast/App.hpp>
#include <gubg/mss.hpp>

namespace proast { 
    bool App::run()
    {
        MSS_BEGIN(bool);
        if (options.print_help)
        {
            std::cout << options.help();
        }
        else
        {
            proast::model::Tree::Config config;
            for (const auto &root: options.roots)
            {
                MSS(model_.add_root(root, config));
            }

            MSS(presenter_.run());
        }
        MSS_END();
    }
} 
