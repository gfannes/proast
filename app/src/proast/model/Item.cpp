#include <proast/model/Item.hpp>
#include <cassert>
#include <cctype>

namespace proast { namespace model { 

    std::string hr(State state)
    {
        switch (state)
        {
            case State::Unclear: return "Unclear";
            case State::Clear: return "Clear";
            case State::Thinking: return "Thinking";
            case State::Designed: return "Designed";
            case State::Implementing: return "Implementing";
            case State::Done: return "Done";
        }
        return "";
    }
    std::optional<State> to_status(const std::string &str)
    {
        if (false) {}
        else if (str == "Unclear") {return State::Unclear;}
        else if (str == "Clear") {return State::Clear;}
        else if (str == "Thinking") {return State::Thinking;}
        else if (str == "Designed") {return State::Designed;}
        else if (str == "Implementing") {return State::Implementing;}
        else if (str == "Done") {return State::Done;}
        return std::nullopt;
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
    std::optional<Priority> to_priority(const std::string &str)
    {
        if (false) {}
        else if (str == "Must") {return Priority::Must;}
        else if (str == "Should") {return Priority::Should;}
        else if (str == "Could") {return Priority::Could;}
        else if (str == "Wont") {return Priority::Wont;}
        return std::nullopt;
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

    std::string key_as_title(const std::string &str)
    {
        auto res = str;

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
    std::string title_as_key(const std::string &str)
    {
        auto res = str;

        for (auto &ch: res)
        {
            //Replace ' ' with '_'
            switch (ch)
            {
                case ' ': ch = '_'; break;
                default: break;
            }

            //Decapitalize
            if ('A' <= ch && ch <= 'Z')
                ch = std::tolower(ch);
        }

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
        os << "[Item](key:" << key << ")(title:" << title << ")(path:" << to_string(path) << ")";
        if (type)
            os << "(type:" << hr(*type) << ")";
        if (priority)
            os << "(priority:" << hr(*priority) << ")";
        if (directory)
            os << "(directory:" << *directory << ")";
        if (content_fp)
            os << "(content_fp:" << *content_fp << ")";
        if (link)
            os << "(link:" << to_string(*link) << ")";
        os << "{" << std::endl;
        for (const auto &desc: description)
            os << "  " << desc << std::endl;
        os << "}";
    }

} } 
