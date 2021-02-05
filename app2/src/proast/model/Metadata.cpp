#include <proast/model/Metadata.hpp>
#include <proast/model/Tree.hpp>
#include <proast/util.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/mss.hpp>
#include <string>

namespace proast { namespace model { 
    bool Metadata::has_local_data() const
    {
        bool b = false;
        auto check = [&](auto &opt){if (opt) b = true;};
        check(my_effort);
        check(my_live);
        check(my_dead);
        check(my_completion_pct);
        check(my_volume_db);
        check(my_impact);
        if (my_tags.size())
            b = true;
        return b;
    }

    double Metadata::effort() const
    {
        double sum = my_effort.value_or(0.0);
        auto aggregate = [&](Node &n)
        {
            sum += n.value.metadata.my_effort.value_or(0.0);
        };
        dependencies.each(aggregate);
        return sum;
    }
    Metadata::Tags Metadata::tags() const
    {
        Tags tags = my_tags;
        auto aggregate = [&](Node &n)
        {
            auto &n_tags = n.value.metadata.my_tags;
            tags.insert(n_tags.begin(), n_tags.end());
        };
        dependencies.each(aggregate);
        return tags;
    }
    void Metadata::stream(gubg::naft::Node &body)
    {
        auto stream_item_2 = [&](auto name, auto &item)
        {
            if (!item) return;
            auto n = body.node(name);
            n.attr("value", *item);
        };
        auto stream_item_3 = [&](auto name, auto &item, auto &&to_str)
        {
            if (!item) return;
            auto n = body.node(name);
            n.attr("value", to_str(*item));
        };
        auto piped = [&](const auto &tags)
        {
            std::string res;
            gubg::OnlyOnce skip_pipe;
            for (const auto &tag: tags)
            {
                if (!skip_pipe())
                    res.push_back('|');
                res += proast::to_utf8(tag);
            }
            return res;
        };

        stream_item_2("Effort", my_effort);
        if (my_tags.size())
        {
            auto n = body.node("Tags");
            n.attr("value", piped(my_tags));
        }
        stream_item_3("Live", my_live, proast::to_utf8);
        stream_item_3("Dead", my_dead, proast::to_utf8);
        stream_item_2("Completion_pct", my_completion_pct);
        stream_item_2("Volume_db", my_volume_db);
        stream_item_2("Impact", my_impact);
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
            else if (tag == "Effort") my_effort = std::stod(value);
            else if (tag == "Volume_db") my_volume_db = std::stod(value);
            else if (tag == "Impact") my_impact = std::stod(value);
            else if (tag == "Completion_pct") my_completion_pct = std::stod(value);
            else if (tag == "Dead") my_dead = proast::to_wstring(value);
            else if (tag == "Live") my_dead = proast::to_wstring(value);
            else if (tag == "Tags")
            {
                for (std::string_view sv{value}; !sv.empty();)
                    if (auto ix = sv.find('|'))
                    {
                        const std::string str = std::string{sv.substr(0, ix)};
                        my_tags.emplace(proast::to_wstring(str));
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
        if (!my_effort) my_effort = other.my_effort;
        if (!my_volume_db) my_volume_db = other.my_volume_db;
        if (!my_impact) my_impact = other.my_impact;
        if (!my_completion_pct) my_completion_pct = other.my_completion_pct;
        if (!my_dead) my_dead = other.my_dead;
        if (!my_live) my_live = other.my_live;
        if (!my_tags.size()) my_tags = other.my_tags;
    }
} } 
