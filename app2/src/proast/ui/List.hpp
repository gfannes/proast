#ifndef HEADER_proast_ui_List_hpp_ALREADY_INCLUDED
#define HEADER_proast_ui_List_hpp_ALREADY_INCLUDED

#include <proast/dto/List.hpp>
#include <ftxui/dom/node.hpp>

namespace proast { namespace ui { 
    class List: public ftxui::Node
    {
    public:
        List(const dto::List::Ptr &list): list_(list) { }

        void ComputeRequirement() override;
        void SetBox(ftxui::Box) override;
        void Render(ftxui::Screen &) override;

    private:
        dto::List::Ptr list_;
        ftxui::Box box_;
    };

    ftxui::Element list(const dto::List::Ptr &);
} } 

#endif
