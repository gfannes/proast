#include <proast/ui/List.hpp>
#include <ftxui/component/component.hpp>

namespace proast { namespace ui { 
    ftxui::Element list(const dto::List::Ptr &list, const std::wstring &str)
    {
        if (list)
            return ftxui::Element(new List(list));
        return ftxui::text(str);
    }
} } 
