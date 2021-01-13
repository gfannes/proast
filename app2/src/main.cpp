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
        }
    private:
        std::wstring str;
    };

    int main(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        proast::Options options;
        MSS(options.parse(argc, argv));
        options.stream(std::cout);

        proast::Tree tree;
        proast::Tree::Config config;
        for (const auto &root: options.roots)
        {
            MSS(tree.add(root, config));
        }

        auto screen = ftxui::ScreenInteractive::Fullscreen();
        MyComponent component;
        component.stop = screen.ExitLoopClosure();
        screen.Loop(&component);
        std::cout << screen.ToString() << std::endl;


        MSS_END();
    }
} 

int main(int argc, const char **argv)
{
    if (!proast::main(argc, argv))
    {
        std::cout << "Error: something went wrong" << std::endl;
        return -1;
    }
    std::cout << "Everything went OK" << std::endl;
    return 0;
}
