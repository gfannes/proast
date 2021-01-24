#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace proast { namespace view { 
    class View: public ftxui::Component
    {
    public:
        struct Events
        {
            virtual ~Events() {}
            virtual void received(wchar_t) = 0;
        };
        Events *events = nullptr;

        std::function<void()> quit;

        void run();

        //ftxui::Component API
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event event) override;

    private:
        std::wstring header_ = L"header";
        std::wstring footer_ = L"footer";
    };
} } 

#endif
