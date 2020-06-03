#include <proast/model/Node.hpp>
#include <gubg/mss.hpp>
#include <sstream>
#include <optional>

namespace proast { namespace model { 

    bool write_markdown(std::string &markdown, const Node_ &node)
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
                if (child.value.link.empty())
                    oss << "* [external](key:" << child.value.key() << ")\n";
                else
                    oss << "* [external](path:" << to_string(child.value.link) << ")\n";
            }
        }

        markdown = oss.str();
        MSS_END();
    }

} } 
