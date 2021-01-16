#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/Tree.hpp>
#include <gubg/mss.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>

namespace proast { 
    class MyComponent: public ftxui::Component
    {
    public:
        double g = 0.0;
        std::function<void()> stop;
        ftxui::Element Render() override
        {
            using namespace ftxui;
            g += 0.1;
            return vbox({
                    hbox({
                            center(text(str)) | border,
                            center(
                                    vbox({
                                        text(L"a"),
                                        text(L"b"),
                                        text(L"c"),
                                        text(L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
                                        })),
                            center(text(L"right")) | border,
                            }) | border,
                    gauge(g) | border,
                    }) | border;
        }
        bool OnEvent(ftxui::Event event) override
        {
            if (event == ftxui::Event::ArrowLeft)
            {
                str = L"LEFT";
                stop();
            }
            else
            {
                auto &input = event.input();
                str.resize(input.size());
                for (auto ix = 0u; ix < str.size(); ++ix)
                    str[ix] = input[0];
            }
            return true;
        }
    private:
        std::wstring str;
    };

    class App
    {
    public:
        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(options_.parse(argc, argv));
            options_.stream(std::cout);
            MSS_END();
        }

        bool prepare()
        {
            MSS_BEGIN(bool);
            proast::Tree::Config config;
            for (const auto &root: options_.roots)
            {
                MSS(tree_.add(root, config));
            }
            MSS_END();
        }

        bool run()
        {
            MSS_BEGIN(bool);

            auto screen = ftxui::ScreenInteractive::Fullscreen();
            MyComponent component;
            component.stop = screen.ExitLoopClosure();
            screen.Loop(&component);

            MSS_END();
        }

    private:
        Options options_;
        Tree tree_;
    };
} 

#endif
