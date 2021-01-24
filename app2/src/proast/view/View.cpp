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
        return vbox({
                text(header) | border,
                hbox({
                        vbox({
                                center(text(L"/0/-1"))    | border,
                                center(text(L"/0"))       | border,
                                center(text(L"/0/1"))     | border,
                                }) | flex,
                        vbox({
                                center(text(L"/0/0/-1"))  | border,
                                center(ui::list(n00, L"/0/0"))     | border,
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
    bool View::OnEvent(ftxui::Event event)
    {
        if (events)
            events->received(event.character());
        return true;
    }

} } 
