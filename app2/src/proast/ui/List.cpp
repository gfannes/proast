#include <proast/ui/List.hpp>

namespace proast { namespace ui { 
    ftxui::Element list(const dto::List::Ptr &list)
    {
        return ftxui::Element(new List(list));
    }
} } 
