#include <proast/model/Markdown.hpp>
#include <proast/log.hpp>
#include <gubg/Strange.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>
#include <sstream>
#include <optional>
#include <functional>
#include <map>

namespace proast { namespace model { namespace markdown { 

    bool read_directory(Node &node, const std::filesystem::path &directory, const Config &config)
    {
        MSS_BEGIN(bool, "");
        L(C(directory));

        node.value.set_key(directory.stem());

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

        MSS_END();
    }

    class Level
    {
    public:
        using Callback = std::function<void()>;
        std::map<unsigned int, Callback> level__cb;

        Level() {}
        ~Level() { set(0); }
        
        unsigned int level() const {return level_;}

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
        std::vector<std::string> description;
    };

    bool read_string(Node &node, const std::string &markdown)
    {
        MSS_BEGIN(bool);

        std::optional<Data> root_data;

        std::optional<Type> child_type;
        std::optional<Priority> child_priority;
        std::optional<Data> child_data;

        std::map<Type, unsigned int> type__count;

        auto add_child_node = [&]()
        {
            assert(!!child_type);
            if (child_data)
            {
                std::cout << "Adding child node " << hr(*child_type) << " " << child_data->title << std::endl;
                auto &child = node.childs.append();
                gubg::naft::Range range{child_data->title};
                if (range.pop_tag("external"))
                {
                    auto attrs = range.pop_attrs();
                    decltype(attrs.find("")) it;
                    auto has = [&](const char *k){it = attrs.find(k); return it != attrs.end();};
                    if (false) {}
                    else if (has("key"))
                    {
                        child.value.set_key(*child_type, it->second);
                    }
                    else if (has("path"))
                    {
                        child.value.set_key(*child_type, it->second);
                        child.value.link = to_path(it->second);
                    }
                }
                else
                {
                    child.value.set_key(*child_type, type__count[*child_type]++);
                    child.value.set_title(child_data->title);
                    child.value.description = child_data->description;
                    if (child_priority)
                        child.value.priority = *child_priority;
                }
            }
            child_data.reset();
        };

        Level bullet;
        bullet.level__cb[1] = [&]()
        {
            if (child_type)
                switch (*child_type)
                {
                    case Type::Requirement:
                    case Type::Design:
                    case Type::Deliverable:
                    case Type::Feature:
                        add_child_node();
                        break;
                    default: break;
                }
        };

        Level section;
        section.level__cb[1] = [&]()
        {
            if (root_data)
            {
                node.value.set_title(root_data->title);
                node.value.description = root_data->description;
            }
            root_data.reset();
        };
        section.level__cb[2] = [&]()
        {
            if (child_type)
                switch (*child_type)
                {
                    case Type::Free:
                        add_child_node();
                        break;
                    default: break;
                }

            bullet.set(0);
            child_type.reset();
        };
        section.level__cb[3] = [&]()
        {
            bullet.set(0);
            child_priority.reset();
        };

        gubg::Strange strange{markdown};

        for (gubg::Strange line; strange.pop_line(line); )
        {
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
                else if (line.str() == "Requirements") {child_type = Type::Requirement;}
                else if (line.str() == "Design")       {child_type = Type::Design;}
                else if (line.str() == "Deliverables") {child_type = Type::Deliverable;}
                else if (line.str() == "Features")     {child_type = Type::Feature;}
                else
                {
                    child_type = Type::Free;
                    child_data.emplace();
                    child_data->title = line.str();
                }
            }
            else
            {
                bool add_newline = false;
                auto add_description_to = [&](auto &data, const auto &str)
                {
                    auto &description = data.description;

                    if (add_newline)
                    {
                        description.emplace_back("");
                        add_newline = false;
                    }

                    if (str.empty())
                        add_newline = (!description.empty() && !description.back().empty());
                    else
                        description.push_back(str);
                };

                if (!child_type)
                {
                    assert(section.level() < 2);

                    if (root_data)
                        add_description_to(*root_data, line.str());
                }
                else
                {
                    assert(section.level() >= 2);

                    auto check_bullets = [&]()
                    {
                        if (line.pop_if("* "))
                        {
                            bullet.set(1);
                            child_data.emplace();
                            child_data->title = line.str();
                        }
                        else if (line.pop_if("  "))
                        {
                            if (child_data)
                                add_description_to(*child_data, line.str());
                        }
                        else
                        {
                            bullet.set(0);

                            if (root_data)
                                add_description_to(*root_data, line.str());
                        }
                    };

                    switch (*child_type)
                    {
                        case Type::Requirement:
                            if (line.pop_if("### "))
                            {
                                if (false) {}
                                else if (line.str() == "Must")   {child_priority = Priority::Must;}
                                else if (line.str() == "Should") {child_priority = Priority::Should;}
                                else if (line.str() == "Could")  {child_priority = Priority::Could;}
                                else if (line.str() == "Wont")   {child_priority = Priority::Wont;}
                                else { MSS(false, log::stream() << "Error: Unknown priority " << line << "\n"); }
                            }
                            else
                                check_bullets();
                            break;

                        case Type::Design:
                        case Type::Deliverable:
                        case Type::Feature:
                            check_bullets();
                            break;

                        case Type::Free:
                            if (child_data)
                                add_description_to(*child_data, line.str());
                            break;
                    }
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

        {
            const auto title = node.value.title();
            if (!title.empty())
                oss << "# " << title << "\n\n";
        }
        for (const auto &desc: node.value.description)
            oss << desc << std::endl;

        std::optional<Type> current_section;
        std::optional<Priority> current_prio;
        for (const auto &child: node.childs.nodes)
        {
            const auto child_type = child.value.type();
            const auto child_prio = child.value.priority;
            bool new_section = false;
            auto stream_section = [&](unsigned int level, const std::string &title)
            {
                oss << '\n' << std::string(level, '#') << ' ' << title << '\n';
                new_section = true;
            };
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
                if (new_section)
                    oss << std::endl;
                for (const auto &desc: child.value.description)
                    oss << desc << std::endl;
            }
            else
            {
                if (new_section)
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
