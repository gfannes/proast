#include <proast/model/Markdown.hpp>
#include <proast/log.hpp>
#include <gubg/Strange.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/mss.hpp>
#include <sstream>
#include <optional>
#include <functional>
#include <map>

namespace proast { namespace model { namespace markdown { 

    bool read_directory_(Node &node, const std::filesystem::path &directory, const Config &config)
    {
        MSS_BEGIN(bool);

        const auto content_fp = [&]()
        {
            const auto content_fp_leaf = config.content_fp_leaf(directory);
            const auto content_fp_nonleaf = config.content_fp_nonleaf(directory);
            return std::filesystem::exists(content_fp_nonleaf) ? content_fp_nonleaf : content_fp_leaf;
        }();

        MSS(std::filesystem::exists(content_fp));

        std::string content;
        MSS(gubg::file::read(content, content_fp));

        MSS(read_string(node, content));

        for (auto &child: node.childs.nodes)
            if (!child.value.is_embedded() && !child.value.link)
            {
                MSS(read_directory_(child, directory/child.value.key(), config));
            }

        MSS_END();
    }
    bool read_directory(Node &node, const std::filesystem::path &directory, const Config &config)
    {
        MSS_BEGIN(bool);

        node.value.set_key(Type::Feature, directory.stem());
        MSS(read_directory_(node, directory, config));

        MSS_END();
    }

    bool write_directory(const std::filesystem::path &parent_directory, const Node &node, const Config &config)
    {
        MSS_BEGIN(bool);

        const auto &item = node.value;

        MSS(!item.is_embedded());

        bool is_leaf = true;
        for (const auto &child: node.childs.nodes)
            if (!child.value.is_embedded())
                is_leaf = false;

        const auto directory = parent_directory/item.key();

        const auto content_fp = is_leaf ? config.content_fp_leaf(directory) : config.content_fp_nonleaf(directory);
        std::filesystem::create_directories(content_fp.parent_path());

        std::string content;
        MSS(write_string(content, node));

        MSS(gubg::file::write(content, content_fp));

        for (const auto &child: node.childs.nodes)
            if (!child.value.is_embedded() && !child.value.link)
            {
                MSS(write_directory(directory, child, config));
            }

        MSS_END();
    }


    class Data
    {
    public:
        std::string title;
        std::vector<std::string> description;
        bool section = false;
        bool bullet = false;
        bool margin = false;

        std::function<void()> process;

        ~Data() {clear();}

        operator bool () const {return valid_;}

        void clear()
        {
            if (valid_)
            {
                if (process)
                    process();
                valid_ = false;
            }
            title.clear();
            description.clear();
            section = false;
            bullet = false;
            margin = false;
            add_newline_ = false;
        }
        void emplace()
        {
            clear();
            valid_ = true;
        }

        void add_description(const std::string &str)
        {
            if (!valid_)
                return;

            if (add_newline_)
            {
                description.emplace_back("");
                add_newline_ = false;
            }

            if (str.empty())
                add_newline_ = (!description.empty() && !description.back().empty());
            else
                description.push_back(str);
        };

    private:
        bool valid_ = false;
        bool add_newline_ = false;
    };

    bool read_string(Node &node, const std::string &markdown)
    {
        MSS_BEGIN(bool);

        std::map<Type, unsigned int> type__count;

        std::optional<Type> type;
        std::optional<Priority> priority;

        Data data;
        data.process = [&]()
        {
            assert(!!type);
            std::cout << "Adding " << hr(*type) << ": \"" << data.title << "\"" << std::endl;

            if (*type == Type::Free && node.childs.empty())
            {
                //data is for node.value itself
                node.value.set_title(data.title);
                node.value.description = data.description;
            }
            else
            {
                //data is for a new child item
                auto &child = node.childs.append();

                gubg::naft::Range range{data.title};
                if (range.pop_tag("external"))
                {
                    auto attrs = range.pop_attrs();
                    decltype(attrs.find("")) it;
                    auto has = [&](const char *k){it = attrs.find(k); return it != attrs.end();};
                    if (false) {}
                    else if (has("key"))
                    {
                        child.value.set_key(*type, it->second);
                    }
                    else if (has("path"))
                    {
                        child.value.set_key(*type, it->second);
                        child.value.link = to_path(it->second);
                    }
                }
                else
                {
                    child.value.set_key(*type, type__count[*type]++);
                    child.value.set_title(data.title);
                    child.value.description = data.description;
                    if (priority)
                        child.value.priority = *priority;
                }
            }
        };

        //Parse the markdown document line-by-line
        gubg::Strange strange{markdown};
        for (gubg::Strange line; strange.pop_line(line); )
        {
            if (line.pop_if("# "))
            {
                type = Type::Free;
                data.emplace();
                data.title = line.str();
            }
            else if (line.pop_if("## "))
            {
                data.clear();

                if (false) {}
                else if (line.str() == "Requirements") {type = Type::Requirement;}
                else if (line.str() == "Design")       {type = Type::Design;}
                else if (line.str() == "Deliverables") {type = Type::Deliverable;}
                else if (line.str() == "Features")     {type = Type::Feature;}
                else
                {
                    type = Type::Free;
                    data.emplace();
                    data.title = line.str();
                }
            }
            else if (!type)
            {
                //We are receiving data before the title: can be dropped for now
            }
            else
            {
                //We know the item type

                switch (*type)
                {
                    case Type::Free:
                        data.add_description(line.str());
                        break;
                    case Type::Requirement:
                        if (line.pop_if("### "))
                        {
                            if (false) {}
                            else if (line.str() == "Must")   {priority = Priority::Must;}
                            else if (line.str() == "Should") {priority = Priority::Should;}
                            else if (line.str() == "Could")  {priority = Priority::Could;}
                            else if (line.str() == "Wont")   {priority = Priority::Wont;}
                            else { MSS(false, log::stream() << "Error: Unknown priority " << line << "\n"); }
                            data.clear();
                        }
                        else if (line.pop_if("* "))
                        {
                            data.emplace();
                            data.bullet = true;
                            data.title = line.str();
                        }
                        else if (data.bullet)
                        {
                            if (line.pop_if("  "))
                                data.add_description(line.str());
                        }
                        break;
                    case Type::Design:
                    case Type::Deliverable:
                    case Type::Feature:
                        if (line.pop_if("### "))
                        {
                            data.emplace();
                            data.section = true;
                            data.title = line.str();
                        }
                        else if (data.section)
                        {
                            //As long as we're in a section: collect description
                            data.add_description(line.str());
                        }
                        else
                        {
                            if (line.pop_if("* "))
                            {
                                data.emplace();
                                data.bullet = true;
                                data.title = line.str();
                            }
                            else if (data.bullet)
                            {
                                if (line.pop_if("  "))
                                    data.add_description(line.str());
                                else if (line.empty())
                                {
                                    data.clear();
                                }
                                else if (!line.empty())
                                {
                                    data.emplace();
                                    data.margin = true;
                                    data.add_description(line.str());
                                }
                            }
                            else if (data.margin)
                            {
                                if (line.empty())
                                    data.clear();
                                else
                                    data.add_description(line.str());
                            }
                            else if (line.empty())
                            {
                                data.clear();
                            }
                            else
                            {
                                data.emplace();
                                data.margin = true;
                                data.add_description(line.str());
                            }
                        }
                        break;
                }
            }
        }

        MSS_END();
    }

    bool write_string(std::string &markdown, const Node &node)
    {
        MSS_BEGIN(bool);
        std::ostringstream oss;

        oss << "<!--" << std::endl;
        oss << "[proast]";
        oss << "(status:" << hr(node.value.status) << ")";
        if (node.value.priority)
            oss << "(prio:" << hr(*node.value.priority) << ")";
        oss << std::endl;
        oss << "-->" << std::endl;

        gubg::OnlyOnce new_section{false};
        auto stream_section = [&](unsigned int level, const std::string &title)
        {
            if (level > 1)
                oss << '\n';
            oss << std::string(level, '#') << ' ' << title << '\n';
            new_section.reset();
        };

        {
            const auto title = node.value.title();
            if (!title.empty())
                stream_section(1, title);
        }
        for (const auto &desc: node.value.description)
        {
            if (new_section())
                oss << std::endl;
            oss << desc << std::endl;
        }

        std::optional<Type> current_section;
        std::optional<Priority> current_prio;
        for (const auto &child: node.childs.nodes)
        {
            const auto child_type = child.value.type();
            const auto child_prio = child.value.priority;
            switch (child_type)
            {
                case Type::Requirement:
                    if (child_type != current_section)
                    {
                        stream_section(2, "Requirements");
                        current_prio.reset();
                    }
                    if (child_prio && child_prio != current_prio)
                        stream_section(3, hr(*child_prio));
                    current_prio = child_prio;
                    break;
                case Type::Design:
                    if (child_type != current_section)
                        stream_section(2, "Design");
                    break;
                case Type::Deliverable:
                    if (child_type != current_section)
                        stream_section(2, "Deliverables");
                    break;
                case Type::Feature:
                    if (child_type != current_section)
                        stream_section(2, "Features");
                    break;
                case Type::Free:
                    break;
            }
            current_section = child_type;

            if (child_type == Type::Free)
            {
                stream_section(2, child.value.title());
                if (new_section())
                    oss << std::endl;
                for (const auto &desc: child.value.description)
                    oss << desc << std::endl;
            }
            else
            {
                if (new_section())
                    oss << std::endl;

                if (child.value.is_embedded())
                {
                    oss << "* " << child.value.title() << std::endl;
                    for (const auto &desc: child.value.description)
                        oss << "  " << desc << std::endl;
                }
                else
                {
                    if (child.value.link)
                        oss << "* [external](path:" << to_string(*child.value.link) << ")\n";
                    else
                        oss << "* [external](key:" << child.value.key() << ")\n";
                }
            }
        }

        markdown = oss.str();
        MSS_END();
    }

} } } 
