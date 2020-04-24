#ifndef HEADER_proast_model_ListBox_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_ListBox_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <initializer_list>

namespace proast { namespace model { 

    class ListBox
    {
    public:
        ListBox(std::initializer_list<std::string> il): items(il) { }

        std::vector<std::string> items;
        int active_ix = -1;

        template <typename Ftor>
        void each_item(Ftor &&ftor) const
        {
            for (std::size_t ix = 0; ix < items.size(); ++ix)
            {
                const bool is_active = (ix == active_ix);
                ftor(items[ix], is_active);
            }
        }

        bool set_active(const std::string &str)
        {
            for (active_ix = 0; active_ix < items.size(); ++active_ix)
                if (str == items[active_ix])
                    return true;

            active_ix = -1;
            return false;
        }

    private:
    };

} } 

#endif
