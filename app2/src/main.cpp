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
        ftxui::Element Render() override
        {
            using namespace ftxui;
            g += 0.1;
            return vbox({
                    hbox({
                            text(L"left") | border,
                            text(L"middle") | border | flex,
                            text(L"right") | border,
                            }),
                    gauge(g) | border,
                    });
        }
    private:
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
