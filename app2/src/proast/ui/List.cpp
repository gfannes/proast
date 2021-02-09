#include <proast/ui/List.hpp>
#include <proast/util.hpp>
#include <ftxui/component/component.hpp>

namespace proast { namespace ui { 
    namespace  { 
        ftxui::Color attention__color(int attention)
        {
            switch (attention)
            {
                case 0: return ftxui::Color::GrayDark;
                case 1: return ftxui::Color::GrayLight;
                case 2: return ftxui::Color::Magenta;
                case 3: return ftxui::Color::Yellow;
                case 4: return ftxui::Color::BlueLight;
                case 5: return ftxui::Color::Red;
            }
            return ftxui::Color::White;
        }
    } 
    void List::ComputeRequirement()
    {
        if (list_)
            requirement_.min_y = list_->items.size()+2;
        requirement_.min_y = height_;
    }
    void List::SetBox(ftxui::Box box)
    {
        box_ = box;
    }
    void List::Render(ftxui::Screen& screen)
    {
        if (!list_)
            return;

        const auto selected_item_ix = list_->ix;
        const auto item_count = list_->items.size();

        const auto y_margin = 2;
        const auto y_size = std::min<int>(box_.y_max-box_.y_min+1-y_margin, item_count);
        const auto x_margin = 0;
        const auto x_size = box_.x_max-box_.x_min+1-x_margin;
        const int mid_row_ix = (y_size-1)/2;
        const int selected_row_ix = (
                selected_item_ix < mid_row_ix
                ? selected_item_ix
                : (
                    (item_count-selected_item_ix) < (y_size-mid_row_ix)
                    ? (y_size-(item_count-selected_item_ix))
                    : mid_row_ix
                  )
                );
        const int offset = selected_item_ix - selected_row_ix;

        {
            const auto name = to_wstring(list_->name.str);
            for (auto col_ix = 0u; col_ix < name.size() && col_ix < x_size; ++col_ix)
            {
                auto &pxl = screen.PixelAt(box_.x_min+col_ix, box_.y_min);
                pxl.character = name[col_ix];
                pxl.bold = list_->name.bold(col_ix);
                pxl.foreground_color = attention__color(list_->name.attention(col_ix));
            }
        }

        for (auto row_ix = 0u; row_ix < y_size; ++row_ix)
        {
            const int item_ix = row_ix + offset;
            if (0 <= item_ix && item_ix < list_->items.size())
            {
                const auto &item = list_->items[item_ix];
                const auto item_str = to_wstring(item.str);
                for (auto col_ix = 0u; col_ix < item_str.size() && col_ix < x_size; ++col_ix)
                {
                    auto &pxl = screen.PixelAt(box_.x_min+x_margin+col_ix, box_.y_min+1+row_ix);
                    pxl.character = item_str[col_ix];
                    pxl.inverted = (item_ix == selected_item_ix);
                    pxl.bold = item.bold(col_ix);
                    pxl.foreground_color = attention__color(item.attention(col_ix));
                }
            }
        }
    }

    ftxui::Element list(const dto::List::Ptr &list, unsigned int height)
    {
        return ftxui::Element(new List(list, height));
    }
} } 
