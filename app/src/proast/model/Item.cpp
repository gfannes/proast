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
            case Type::Deliverable: return "Deliverable";
            case Type::Free: return "Free";
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

    namespace my { 
        void append(std::string &str, Type type)
        {
            switch (type)
            {
                case Type::Requirement: str.push_back('@'); break;
                case Type::Design:      str.push_back('_'); break;
                case Type::Deliverable: str.push_back('%'); break;
                default: break;
            }
        }
    } 

    std::string Item::key() const
    {
        if (link)
            return to_string(*link);
        return key_;
    }
    void Item::set_key(const std::string &key)
    {
        type_ = Type::Free;
        if (!key.empty())
            switch (key_[0])
            {
                case '@': type_ = Type::Requirement;
                case '_': type_ = Type::Design;
                case '%': type_ = Type::Deliverable;
                default: break;
            }

        key_ = key;
    }
    void Item::set_key(Type type, const std::string &stem)
    {
        type_ = type;
        key_.clear();
        my::append(key_, type);
        key_ += stem;
    }
    void Item::set_key(Type type, unsigned int ix)
    {
        type_ = type;
        key_.clear();
        my::append(key_, type);
        key_.push_back('#');
        key_ += std::to_string(ix);
    }

    std::string Item::stem() const
    {
        switch (type())
        {
            case Type::Requirement:
            case Type::Design:
            case Type::Deliverable:
                assert(!key_.empty());
                return key_.substr(1);

            case Type::Feature:
                return key_;
        }
        return key_;
    }

    const std::string &Item::title() const
    {
        return title_;
    }
    std::string Item::key_as_title() const
    {
        auto res = stem();

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
    void Item::set_title(const std::string &str)
    {
        title_ = str;
    }

    Type Item::type() const
    {
        return type_;
    }

    bool Item::is_embedded() const
    {
        if (link)
            return false;

        char ch = '#';

        switch (type())
        {
            case Type::Requirement:
            case Type::Design:
            case Type::Deliverable:
                if (key_.size() >= 2)
                    ch = key_[1];
                break;

            case Type::Feature:
            case Type::Free:
                if (key_.size() >= 1)
                    ch = key_[0];
                break;
        }

        return ch == '#';
    }

    void Item::stream(std::ostream &os) const
    {
        os << "[Item](key:" << key() << ")(title:" << title() << ")(type:" << hr(type_) << "){" << std::endl;
        for (const auto &desc: description)
            os << "  " << desc << std::endl;
        os << "}";
    }

} } 
