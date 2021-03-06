#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <proast/ui/List.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace proast { namespace view { 
    class View: public ftxui::Component
    {
    public:
        struct Events
        {
            virtual ~Events() {}
            virtual void received(wchar_t, bool alt) = 0;
        };
        Events *events = nullptr;

        std::string header;
        std::string footer;
        proast::dto::List::Ptr metadata;
        proast::dto::List::Ptr n0;
        proast::dto::List::Ptr details;
        proast::dto::List::Ptr n0a;
        proast::dto::List::Ptr n00;
        proast::dto::List::Ptr n0b;
        proast::dto::List::Ptr n00a;
        proast::dto::List::Ptr n000;
        proast::dto::List::Ptr n00b;

        std::function<void()> quit;

        void run();

        void set_header(const std::string &);

        //ftxui::Component API
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event event) override;

    private:
        std::string prev_input_;
    };
} } 

#endif
