#ifndef HEADER_proast_view_Main_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Main_hpp_ALREADY_INCLUDED

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace proast { namespace view { 
    class Main: public ftxui::Component
    {
    public:
        std::function<void()> stop;

        std::wstring header = L"header";
        std::wstring footer = L"footer";

        ftxui::Element Render() override
        {
            using namespace ftxui;
            auto list = dto::List::create();
            list->items.emplace_back(L"zero");
            list->items.emplace_back(L"one");
            list->items.emplace_back(L"two");
            list->items.emplace_back(L"three");
            list->items.emplace_back(L"four");
            list->items.emplace_back(L"five");
            list->items.emplace_back(L"six");
            list->items.emplace_back(L"seven");
            return vbox({
                    text(header) | border,
                    hbox({
                            vbox({
                                    ui::list(list) | border,
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

} } 

#endif
