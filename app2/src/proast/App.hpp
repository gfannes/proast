#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/Tree.hpp>
#include <gubg/mss.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <memory>

namespace proast { 
    namespace md { 
        struct Data
        {
            using Ptr = std::shared_ptr<Data>;
            static Ptr create(){return Ptr{new Data};}

            std::vector<std::wstring> items;
            std::size_t ix = 0;
        };
    } 
    namespace ui { 
        class List: public ftxui::Node
        {
        public:
            List(const md::Data::Ptr &data): data_(data)
            {
            }
            void ComputeRequirement() override
            {
                requirement_.min_y = 5;
            }
            void SetBox(ftxui::Box box) override
            {
                box_ = box;
            }
            void Render(ftxui::Screen& screen) override
            {
                const auto y_size = box_.y_max-box_.y_min+1;
                const auto x_size = box_.x_max-box_.x_min+1;
                for (auto iy = 0u; iy < data_->items.size() && iy < y_size; ++iy)
                {
                    const auto &item = data_->items[iy];
                    for (auto ix = 0u; ix < item.size(); ++ix)
                    {
                        auto &pxl = screen.PixelAt(box_.x_min+ix, box_.y_min+iy);
                        pxl.character = item[ix];
                        pxl.inverted = (iy == data_->ix);
                    }
                }
            }
        private:
            md::Data::Ptr data_;
            ftxui::Box box_;
        };
        ftxui::Element list(const md::Data::Ptr &data)
        {
            return ftxui::Element(new List(data));
        }
    } 
    class MyComponent: public ftxui::Component
    {
    public:
        std::function<void()> stop;

        std::wstring header = L"header";
        std::wstring footer = L"footer";

        ftxui::Element Render() override
        {
            using namespace ftxui;
            auto data = md::Data::create();
            data->items.emplace_back(L"zero");
            data->items.emplace_back(L"one");
            data->items.emplace_back(L"two");
            data->items.emplace_back(L"three");
            data->items.emplace_back(L"four");
            data->items.emplace_back(L"five");
            data->items.emplace_back(L"six");
            data->items.emplace_back(L"seven");
            return vbox({
                    text(header) | border,
                    hbox({
                            vbox({
                                    ui::list(data) | border,
                                    center(text(L"/0/-1"))    | border,
                                    center(text(L"/0"))       | border,
                                    center(text(L"/0/1"))     | border,
                                    }) | flex,
                            vbox({
                                    center(text(L"/0/0/-1"))  | border,
                                    center(text(L"/0/0"))     | border,
                                    center(text(L"/0/0/1"))   | border,
                                    }) | flex,
                            vbox({
                                    center(text(L"/0/0/0"))   | border,
                                    center(text(L"metadata")) | border,
                                    }) | flex,
                            }) | flex,
                    text(footer) | border,
                    });
        }
        bool OnEvent(ftxui::Event event) override
        {
            if (event.is_character())
            {
                switch (event.character())
                {
                    case L'q':
                        stop();
                        break;
                }
            }
            else
            {
                /* auto &input = event.input(); */
                /* str.resize(input.size()); */
                /* for (auto ix = 0u; ix < str.size(); ++ix) */
                /*     str[ix] = input[0]; */
            }
            return true;
        }
    private:
    };

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
                MyComponent component;
                component.stop = screen.ExitLoopClosure();
                screen.Loop(&component);
            }
            MSS_END();
        }

    private:
        Options options_;
        Tree tree_;
    };
} 

#endif
