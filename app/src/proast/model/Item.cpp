#include <proast/model/Item.hpp>
#include <cassert>
#include <cctype>

namespace proast { namespace model { 

    std::string hr(Status status)
    {
        switch (status)
        {
            case Status::Todo: return "todo";
            case Status::InDesign: return "design";
            case Status::Designed: return "designed";
            case Status::Implementing: return "implementing";
            case Status::Implemented: return "implemented";
            case Status::Done: return "done";
        }
        return "";
    }
    std::string hr(Priority prio)
    {
        switch (prio)
        {
            case Priority::Must: return "Must";
            case Priority::Should: return "Should";
            case Priority::Could: return "Could";
            case Priority::Wont: return "Wont";
        }
        return "";
    }
    std::string hr(Type type)
    {
        switch (type)
        {
            case Type::Feature: return "Feature";
            case Type::Requirement: return "Requirement";
            case Type::Design: return "Design";
            case Type::Free: return "Free";
            case Type::File: return "File";
            case Type::Directory: return "Directory";
        }
        return "";
    }
    std::string hr(Style style)
    {
        switch (style)
        {
            case Style::Title: return "Title";
            case Style::Section: return "Section";
            case Style::Bullet: return "Bullet";
            case Style::Margin: return "Margin";
        }
        return "";
    }

    std::string Item::key_as_title() const
    {
        auto res = key;

        //Replace '_' with ' '
        for (auto &ch: res)
            switch (ch)
            {
                case '_': ch = ' '; break;
                default: break;
            }

        //Capitalize
        if (!res.empty())
            res[0] = std::toupper(res[0]);

        return res;
    }

    bool Item::is_embedded() const
    {
        if (link)
            return false;

        char ch = '#';

        if (!key.empty())
            ch = key[0];

        return ch == '#';
    }

    void Item::stream(std::ostream &os) const
    {
        os << "[Item](key:" << key << ")(title:" << title << ")";
        if (type)
            os << "(type:" << hr(*type) << ")";
        os << "{" << std::endl;
        for (const auto &desc: description)
            os << "  " << desc << std::endl;
        os << "}";
    }

} } 
