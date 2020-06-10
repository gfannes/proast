#ifndef HEADER_proast_model_ListBox_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_ListBox_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>

namespace proast { namespace presenter { 

    class ListBox
    {
    public:
        struct Entry
        {
            std::string text;
            std::string meta;
            unsigned int attention = 0;
            bool done = false;

            Entry() {}
            Entry(const std::string &text): text(text) {}
            Entry(const std::string &text, unsigned int attention): text(text), attention(attention) {}
            Entry(const std::string &text, const std::string &meta, unsigned int attention): text(text), meta(meta), attention(attention) {}
        };

        ListBox() {}
        ListBox(std::initializer_list<std::string> il): entries(il.size())
        {
            auto dst = entries.begin();
            for (const std::string &str: il)
                *dst++ = str;
        }

        void clear() {*this = ListBox{};}

        std::vector<Entry> entries;
        int active_ix = -1;

        template <typename Ftor>
        void each_item(Ftor &&ftor) const
        {
            for (std::size_t ix = 0; ix < entries.size(); ++ix)
            {
                const auto &entry = entries[ix];
                const bool is_active = (ix == active_ix);
                ftor(entry.text, entry.meta, entry.attention, entry.done, is_active);
            }
        }

        bool set_active(int ix)
        {
            if (ix < 0 || ix >= entries.size())
            {
                active_ix = -1;
                return false;
            }
            active_ix = ix;
            return true;
        }

        std::size_t meta_size() const
        {
            std::size_t size = 0;
            for (const auto &entry: entries)
                size = std::max(size, entry.meta.size());
            return size;
        }

    private:
    };

} } 

#endif
