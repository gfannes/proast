#include <proast/view/View.hpp>
#include <proast/ui/List.hpp>

namespace proast { namespace view { 
    void View::run()
    {
        auto screen = ftxui::ScreenInteractive::Fullscreen();
        quit = screen.ExitLoopClosure();
        screen.Loop(this);
    }

    //ftxui::Component API
    ftxui::Element View::Render()
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
                text(header_) | border,
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
                text(footer_) | border,
                });
    }
    bool View::OnEvent(ftxui::Event event)
    {
        if (events)
            events->received(event.character());
        return true;
    }

} } 
