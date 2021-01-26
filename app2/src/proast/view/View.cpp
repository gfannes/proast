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
                                ui::list(na) | border,
                                ui::list(n0) | border,
                                ui::list(nb) | border,
                                }) | flex,
                        vbox({
                                ui::list(n0a) | border,
                                ui::list(n00) | border,
                                ui::list(n0b) | border,
                                }) | flex,
                        vbox({
                                ui::list(n00a) | border,
                                ui::list(n000) | border,
                                ui::list(n00b) | border,
                                }) | flex,
                        }) | flex,
                text(footer) | border,
                });
    }
    bool View::OnEvent(ftxui::Event event)
    {
        if (events)
        {
            if (false) {}
            else if (event == ftxui::Event::ArrowLeft)  events->received(L'\u2190');
            else if (event == ftxui::Event::ArrowUp)    events->received(L'\u2191');
            else if (event == ftxui::Event::ArrowRight) events->received(L'\u2192');
            else if (event == ftxui::Event::ArrowDown)  events->received(L'\u2193');
            else events->received(event.character());
        }
        return true;
    }

} } 
