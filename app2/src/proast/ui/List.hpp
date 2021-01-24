#ifndef HEADER_proast_ui_List_hpp_ALREADY_INCLUDED
#define HEADER_proast_ui_List_hpp_ALREADY_INCLUDED

#include <proast/dto/List.hpp>
#include <ftxui/dom/node.hpp>

namespace proast { namespace ui { 
    class List: public ftxui::Node
    {
    public:
        List(const dto::List::Ptr &list): list_(list)
        {
        }
        void ComputeRequirement() override
        {
            requirement_.min_y = 5;
        }
        void SetBox(ftxui::Box box) override
        {
            box_ = box;
        }
        void Render(ftxui::Screen& screen) override
        {
            const auto y_size = box_.y_max-box_.y_min+1;
            const auto x_size = box_.x_max-box_.x_min+1;
            for (auto iy = 0u; iy < list_->items.size() && iy < y_size; ++iy)
            {
                const auto &item = list_->items[iy];
                for (auto ix = 0u; ix < item.size(); ++ix)
                {
                    auto &pxl = screen.PixelAt(box_.x_min+ix, box_.y_min+iy);
                    pxl.character = item[ix];
                    pxl.inverted = (iy == list_->ix);
                }
            }
        }
    private:
        dto::List::Ptr list_;
        ftxui::Box box_;
    };

    ftxui::Element list(const dto::List::Ptr &, const std::wstring &);
} } 

#endif
