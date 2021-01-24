#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/presenter/Presenter.hpp>
#include <proast/ui/List.hpp>
#include <gubg/mss.hpp>
#include <iostream>
#include <memory>

namespace proast { 
    class App
    {
    public:
        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(options_.parse(argc, argv));
            if (options_.verbose >= 1)
                options_.stream(std::cout);
            MSS_END();
        }

        bool prepare()
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
            }
            MSS_END();
        }

        bool run()
        {
            MSS_BEGIN(bool);
            if (options_.print_help)
            {
            }
            else
            {
                presenter_.run();
            }
            MSS_END();
        }

    private:
        Options options_;
        model::Model model_;
        view::View view_;
        presenter::Presenter presenter_{model_, view_};
    };
} 

#endif
