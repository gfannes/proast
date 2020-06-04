#include <proast/model/Node.hpp>
#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>
#include <sstream>
#include <optional>
#include <functional>
#include <map>

namespace proast { namespace model { 

    class Level
    {
    public:
        using Callback = std::function<void()>;
        std::map<unsigned int, Callback> level__cb;

        Level() {}
        ~Level() { set(0); }

        void set(unsigned int level)
        {
            if (level_ < level)
            {
                //Level should be increased: no callbacks are triggered
                //since no level is left or reset
                level_ = level;
                return;
            }

            while (true)
            {
                std::cout << C(level)C(level_) << std::endl;
                //Even if the level remains the same, we are triggering
                //callbacks since that means resetting a level
                auto it = level__cb.find(level_);
                if (it != level__cb.end() && it->second)
                    it->second();

                if (level_ == level)
                    //We reached the expected level
                    return;

                --level_;
            }
        }

    private:
        unsigned int level_ = 0;
    };

    struct Data
    {
        std::string title;
        std::ostringstream description;
    };

    bool read_markdown(Node &node, const std::string &markdown)
    {
        MSS_BEGIN(bool);

        std::optional<Data> root_data;
        std::optional<Type> type;
        std::optional<Data> child_data;

        std::map<Type, unsigned int> type__count;

        Level bullet;
        bullet.level__cb[1] = [&]()
        {
            std::cout << "Triggered bullet" << std::endl;
            if (child_data && type)
            {
                std::cout << "Adding new time" << std::endl;
                auto &child = node.childs.append();
                child.value.set_key(*type, type__count[*type]++);
                child.value.set_title(child_data->title);
                child.value.set_description(child_data->description.str());
            }
            child_data.reset();
        };

        Level section;
        section.level__cb[1] = [&]()
        {
            if (root_data)
            {
                node.value.set_title(root_data->title);
                node.value.set_description(root_data->description.str());
            }
            root_data.reset();
        };
        section.level__cb[2] = [&]()
        {
            bullet.set(0);
            type.reset();
        };

        gubg::Strange strange{markdown};

        for (gubg::Strange line; strange.pop_line(line); )
        {
            std::cout << "(" << line << ")" << std::endl;

            if (false) {}
            else if (line.pop_if("# "))
            {
                section.set(1);
                root_data.emplace();
                root_data->title = line.str();
            }
            else if (line.pop_if("## "))
            {
                //Close the root_data collection
                section.set(1);
                section.level__cb[1] = Level::Callback{};

                section.set(2);

                if (false) {}
                else if (line.str() == "Requirements") {type = Type::Requirement;}
                else if (line.str() == "Design") {type = Type::Design;}
                else if (line.str() == "Deliverables") {type = Type::Deliverable;}
                else if (line.str() == "Features") {type = Type::Feature;}
            }
            else if (line.pop_if("###"))
            {
                //Close the root_data collection
                section.set(1);
                section.level__cb[1] = Level::Callback{};

                type.reset();
            }
            else if (line.pop_if("* "))
            {
                bullet.set(1);
                child_data.emplace();
                child_data->title = line.str();
            }
            else if (line.pop_if("  "))
            {
                if (child_data)
                    child_data->description << line.str() << std::endl;
            }
            else
            {
                bullet.set(0);

                if (root_data)
                    root_data->description << line.str() << std::endl;
            }
        }

        MSS_END();
    }

    bool write_markdown(std::string &markdown, const Node &node)
    {
        MSS_BEGIN(bool);
        std::ostringstream oss;

        oss << "<!--" << std::endl;
        oss << "[proast]";
        oss << "(status:" << hr(node.value.status) << ")";
        oss << std::endl;
        oss << "-->" << std::endl;

        {
            const auto title = node.value.title();
            if (!title.empty())
                oss << "# " << title << "\n\n";
        }
        {
            const auto description = node.value.description();
            if (!description.empty())
                oss << description << std::endl;
        }

        std::optional<Type> current_section;
        for (const auto &child: node.childs.nodes)
        {
            const auto child_type = child.value.type();
            if (!current_section || child_type != current_section)
            {
                current_section = child_type;
                switch (*current_section)
                {
                    case Type::Requirement:
                        oss << "\n## Requirements\n\n";
                        break;
                    case Type::Design:
                        oss << "\n## Design\n\n";
                        break;
                    case Type::Deliverable:
                        oss << "\n## Deliverables\n\n";
                        break;
                    case Type::Feature:
                        oss << "\n## Features\n\n";
                        break;
                    default: MSS(false); break;
                }
            }

            if (child.value.is_embedded())
            {
                oss << "* " << child.value.title() << std::endl;
                const auto description = child.value.description();
                if (!description.empty())
                    oss << "  * " << description << std::endl;
            }
            else
            {
                if (child.value.link)
                    oss << "* [external](path:" << to_string(*child.value.link) << ")\n";
                else
                    oss << "* [external](key:" << child.value.key() << ")\n";
            }
        }

        markdown = oss.str();
        MSS_END();
    }

} } 
