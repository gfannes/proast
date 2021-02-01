#include <proast/view/View.hpp>
#include <proast/unicode.hpp>
#include <proast/ui/List.hpp>
#include <proast/log.hpp>

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
        const auto small_frac = 0.2;
        const auto large_frac = 1.0-2*small_frac;
        const auto height = 50;
        const unsigned int small_height = small_frac*height;
        const unsigned int large_height = large_frac*height;
        return vbox({
                text(header) | border,
                hbox({
                        vbox({
                                ui::list(metadata, small_height),
                                ui::list(n0, large_height),
                                ui::list(details, small_height),
                                }) | flex,
                        vbox({
                                ui::list(n0a, small_height),
                                ui::list(n00, large_height),
                                ui::list(n0b, small_height),
                                }) | flex,
                        vbox({
                                ui::list(n00a, small_height),
                                ui::list(n000, large_height),
                                ui::list(n00b, small_height),
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
            else if (event == ftxui::Event::ArrowLeft)  events->received(ArrowLeft);
            else if (event == ftxui::Event::ArrowUp)    events->received(ArrowUp);
            else if (event == ftxui::Event::ArrowRight) events->received(ArrowRight);
            else if (event == ftxui::Event::ArrowDown)  events->received(ArrowDown);
            else if (event == ftxui::Event::Return)     events->received(Return);
            else if (event == ftxui::Event::Escape)     events->received(Escape);
            else events->received(event.character());
        }
        return true;
    }

} } 