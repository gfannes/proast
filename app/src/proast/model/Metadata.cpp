#include <proast/model/Metadata.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/mss.hpp>
#include <string>
#include <set>

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
        check(state);
        if (done)
            b = true;
        if (order_sequential)
            b = true;
        if (tags.size())
            b = true;
        return b;
    }

    double Metadata::get_volume_db() const
    {
        return volume_db.value_or(-20.0);
    }
    double Metadata::get_impact() const
    {
        return impact.value_or(1.0);
    }
    double Metadata::get_age() const
    {
        //TODO: compute actual age as new-live
        return 1.0;
    }

    void Metadata::stream(gubg::naft::Node &body)
    {
        auto stream_item = [&](auto name, const auto &item)
        {
            auto n = body.node(name);
            n.attr("value", item);
        };
        auto stream_item_if = [&](auto name, auto &item)
        {
            if (!item) return;
            stream_item(name, *item);
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

        stream_item_if("Effort", effort);
        if (tags.size())
        {
            auto n = body.node("Tags");
            n.attr("value", piped(tags));
        }
        stream_item_if("Live", live);
        stream_item_if("Due", due);
        stream_item_if("Dead", dead);
        stream_item_if("Completion_pct", completion_pct);
        stream_item_if("Volume_db", volume_db);
        stream_item_if("Impact", impact);
        if (state)
        {
            auto n = body.node("State");
            n.attr("value", model::to_string(*state));
            n.attr("done", (done ? "yes" : "no"));
        }
        if (order_sequential)
        {
            auto n = body.node("Order");
            n.attr("value", (order_sequential ? "Sequential" : "Random"));
        }
    }
    bool Metadata::parse(gubg::naft::Range &range)
    {
        MSS_BEGIN(bool);
        std::string key, value;
        static const std::set<std::string> yesses = {"yes", "Yes", "y", "Y", "1", "true", "True"};
        auto yesno = [&](const auto &str){
            return yesses.count(str) > 0;
        };
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
            else if (tag == "State")
            {
                state = to_State(value);
                MSS(range.pop_attr(key, value));
                MSS(key == "done");
                done = yesno(value);
            }
            else if (tag == "Order")
            {
                if (false) {}
                else if (value == "Sequential")
                    order_sequential = true;
                else if (value == "Random")
                    order_sequential = false;
                else
                    MSS(false);
            }
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
        if (!state) state = other.state;
        done = other.done;
        order_sequential = other.order_sequential;
        if (!tags.size()) tags = other.tags;
    }
} } 
