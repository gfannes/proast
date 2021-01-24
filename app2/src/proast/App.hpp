#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/Tree.hpp>
#include <proast/ui/List.hpp>
#include <proast/view/Main.hpp>
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
                proast::Tree::Config config;
                for (const auto &root: options_.roots)
                {
                    MSS(tree_.add(root, config));
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
                auto screen = ftxui::ScreenInteractive::Fullscreen();
                view::Main main_view;
                main_view.stop = screen.ExitLoopClosure();
                screen.Loop(&main_view);
            }
            MSS_END();
        }

    private:
        Options options_;
        Tree tree_;
    };
} 

#endif
