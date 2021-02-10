#include <proast/model/Metadata.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/mss.hpp>
#include <string>

namespace proast { namespace model { 
    bool Metadata::has_local_data() const
    {
        bool b = false;
        auto check = [&](auto &opt){if (opt) b = true;};
        check(effort);
        check(live);
        check(due);
        check(dead);
        check(completion_pct);
        check(volume_db);
        check(impact);
        if (tags.size())
            b = true;
        return b;
    }

    void Metadata::stream(gubg::naft::Node &body)
    {
        auto stream_item_2 = [&](auto name, auto &item)
        {
            if (!item) return;
            auto n = body.node(name);
            n.attr("value", *item);
        };
        auto piped = [&](const auto &tags)
        {
            std::string res;
            gubg::OnlyOnce skip_pipe;
            for (const auto &tag: tags)
            {
                if (!skip_pipe())
                    res.push_back('|');
                res += tag;
            }
            return res;
        };

        stream_item_2("Effort", effort);
        if (tags.size())
        {
            auto n = body.node("Tags");
            n.attr("value", piped(tags));
        }
        stream_item_2("Live", live);
        stream_item_2("Due", due);
        stream_item_2("Dead", dead);
        stream_item_2("Completion_pct", completion_pct);
        stream_item_2("Volume_db", volume_db);
        stream_item_2("Impact", impact);
    }
    bool Metadata::parse(gubg::naft::Range &range)
    {
        MSS_BEGIN(bool);
        std::string key, value;
        for (std::string tag; range.pop_tag(tag);)
        {
            MSS(range.pop_attr(key, value));
            MSS(key == "value");
            if (false) {}
            else if (tag == "Effort") effort = std::stod(value);
            else if (tag == "Volume_db") volume_db = std::stod(value);
            else if (tag == "Impact") impact = std::stod(value);
            else if (tag == "Completion_pct") completion_pct = std::stod(value);
            else if (tag == "Live") dead = value;
            else if (tag == "Due") due = value;
            else if (tag == "Dead") dead = value;
            else if (tag == "Tags")
            {
                for (std::string_view sv{value}; !sv.empty();)
                    if (auto ix = sv.find('|'))
                    {
                        const std::string str = std::string{sv.substr(0, ix)};
                        tags.emplace(str);
                        sv.remove_prefix(str.size());

                        if (ix != sv.npos)
                            sv.remove_prefix(1);
                    }
                
            }
        }
        MSS_END();
    }

    void Metadata::set_when_unset(const Metadata &other)
    {
        if (!effort) effort = other.effort;
        if (!volume_db) volume_db = other.volume_db;
        if (!impact) impact = other.impact;
        if (!completion_pct) completion_pct = other.completion_pct;
        if (!dead) dead = other.dead;
        if (!live) live = other.live;
        if (!due) due = other.due;
        if (!tags.size()) tags = other.tags;
    }
} } 
