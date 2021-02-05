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
        check(my_tags);
        check(my_live);
        check(my_dead);
        check(my_completion_pct);
        check(my_volume_db);
        check(my_impact);
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
        Tags tags;
        auto aggregate = [&](Node &n)
        {
            if (auto &n_tags = n.value.metadata.my_tags)
                tags.insert(n_tags->begin(), n_tags->end());
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
        stream_item_3("Tags", my_tags, piped);
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
            else if (tag == "Effort")
                my_effort = std::stod(value);
        }
        MSS_END();
    }

    void Metadata::set_when_unset(const Metadata &other)
    {
        if (!my_effort) my_effort = other.my_effort;
    }
} } 
