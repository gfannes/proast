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
        key_ = key;
    }
    void Item::set_key(Type type, const std::string &stem)
    {
        key_.clear();
        my::append(key_, type);
        key_ += stem;
    }
    void Item::set_key(Type type, unsigned int ix)
    {
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

    std::string Item::title() const
    {
        if (!title_.empty())
            return title_;

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

    std::string Item::description() const
    {
        return description_;
    }
    void Item::set_description(const std::string &str)
    {
        description_ = str;
    }

    Type Item::type() const
    {
        if (key_.empty())
            return Type::Feature;

        switch (key_[0])
        {
            case '@': return Type::Requirement;
            case '_': return Type::Design;
            case '%': return Type::Deliverable;
            default: break;
        }

        return Type::Feature;
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
                if (key_.size() >= 1)
                    ch = key_[0];
                break;
        }

        return ch == '#';
    }

    void Item::stream(std::ostream &os) const
    {
        os << "[Item](key:" << key() << ")(title:" << title() << "){" << std::endl;
        os << description();
        os << "}";
    }

} } 
