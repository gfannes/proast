#include <proast/App.hpp>
#include <gubg/mss.hpp>

namespace proast { 
    bool App::parse(int argc, const char **argv)
    {
        MSS_BEGIN(bool);
        MSS(options_.parse(argc, argv));
        if (options_.verbose >= 1)
            options_.stream(std::cout);
        MSS_END();
    }

    bool App::run()
    {
        MSS_BEGIN(bool);
        if (options_.print_help)
        {
            std::cout << options_.help();
        }
        else
        {
            proast::model::Tree::Config config;
            for (const auto &root: options_.roots)
            {
                MSS(model_.add_root(root, config));
            }

            presenter_.run();
        }
        MSS_END();
    }
} 
