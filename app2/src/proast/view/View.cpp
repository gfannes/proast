#include <proast/view/View.hpp>
#include <proast/ui/List.hpp>
#include <proast/util.hpp>

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
        log([](auto &os){os << "Rendering" << std::endl;});
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
                                ui::list(na, small_height) | border,
                                ui::list(n0, large_height) | border,
                                ui::list(nb, small_height) | border,
                                }) | flex,
                        vbox({
                                ui::list(n0a, small_height) | border,
                                ui::list(n00, large_height) | border,
                                ui::list(n0b, small_height) | border,
                                }) | flex,
                        vbox({
                                ui::list(n00a, small_height) | border,
                                ui::list(n000, large_height) | border,
                                ui::list(n00b, small_height) | border,
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
            else if (event == ftxui::Event::Return)     events->received(L'\n');
            else events->received(event.character());
        }
        return true;
    }

} } 
