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
            MSS(model_.set_home_dir(options.home_dir));

            proast::model::Model::Config config;
            for (const auto &root: options.roots)
                if (model_.add_root(root, config))
                    std::cout << "Error: could not add " << root << std::endl;

            MSS(presenter_.run());
        }

        MSS_END();
    }
} 
