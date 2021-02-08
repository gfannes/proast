#include <proast/view/View.hpp>
#include <proast/unicode.hpp>
#include <proast/util.hpp>
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
        auto s = log::Scope{"View::Render()"};

        using namespace ftxui;
        const auto small_frac = 0.2;
        const auto large_frac = 1.0-2*small_frac;
        const auto height = 50;
        const unsigned int equal_height = height/3;
        const unsigned int small_height = small_frac*height;
        const unsigned int large_height = large_frac*height;
        return vbox({
                text(to_wstring(header)) | border,
                hbox({
                        vbox({
                                ui::list(metadata, equal_height),
                                ui::list(n0,       equal_height),
                                ui::list(details,  equal_height),
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
                text(to_wstring(footer)) | border,
                });
    }
    bool View::OnEvent(ftxui::Event event)
    {
        auto s = log::Scope{"View::OnEvent()", [&](auto &hdr){
            hdr.attr("input.size", event.input().size()).attr("is_character", event.is_character());
            for (auto ch: event.input())
                hdr.attr("ch", (int)ch);
        }};

        if (events)
        {
            if (false) {}
            else if (event == ftxui::Event::ArrowLeft)  events->received(ArrowLeft, false);
            else if (event == ftxui::Event::ArrowUp)    events->received(ArrowUp, false);
            else if (event == ftxui::Event::ArrowRight) events->received(ArrowRight, false);
            else if (event == ftxui::Event::ArrowDown)  events->received(ArrowDown, false);
            else if (event == ftxui::Event::Return)     events->received(Return, false);
            else if (event == ftxui::Event::Escape)     events->received(Escape, false);
            else if (event == ftxui::Event::Backspace)  events->received(Backspace, false);
            else
            {
                //When alt-key is pressed, it is signalled as 27-X. With key-repeat together with alt
                //event.input() is sometimes a bit wrong: `27-X-27` `X` `27-X`. This is why we detect
                //alt ourselves and keep track of an already received alt.
                prev_input_ += event.input();
                while (!prev_input_.empty())
                {
                    if (prev_input_[0] == 27)//Alt
                    {
                        if (prev_input_.size() < 2)
                            //We did not receive the actual key yet
                            break;
                        events->received(prev_input_[1], true);
                        prev_input_ = prev_input_.substr(2);
                    }
                    else
                    {
                        events->received(prev_input_[0], false);
                        prev_input_ = prev_input_.substr(1);
                    }
                }
            }
        }
        return true;
    }

} } 
