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

        node.value.directory = directory;

        const auto content_fp = [&]()
        {
            const auto content_fp_leaf = config.content_fp_leaf(directory);
            const auto content_fp_nonleaf = config.content_fp_nonleaf(directory);
            return std::filesystem::exists(content_fp_nonleaf) ? content_fp_nonleaf : content_fp_leaf;
        }();

        MSS(std::filesystem::exists(content_fp), log::stream() << "Error: content file " << content_fp << " does not exist for " << directory << std::endl);

        std::string content;
        MSS(gubg::file::read(content, content_fp));
        node.value.content_fp = content_fp;

        MSS(read_string(node, content));

        std::map<std::string, std::optional<std::size_t>> key__ix;

        for (auto ix = 0u; ix < node.childs.size(); ++ix)
        {
            auto &child = node.childs.nodes[ix];
            if (!child.value.is_embedded() && !child.value.link)
                key__ix[child.value.key] = ix;
        }

        if (std::filesystem::exists(directory))
            for (const auto &entry: std::filesystem::directory_iterator{directory})
            {
                const auto fp = entry.path();
                const bool file_dir = std::filesystem::is_regular_file(fp);
                const auto stem = file_dir ? fp.stem().string() : fp.filename().string();
                const auto extension = file_dir ? fp.extension().string() : std::string();

                if (key__ix.count(stem) > 0)
                    //This already exists
                    continue;
                if (stem.empty())
                    continue;
                if (stem[0] == '.')
                    continue;

                if (file_dir)
                {
                    if (extension != config.extension())
                        continue;
                    if (fp.filename() == config.index_filename())
                        continue;
                }

                auto &child = node.childs.append();
                child.value.key = fp.filename();
                child.value.type = file_dir ? Type::File : Type::Directory;
                child.value.content_fp = fp;
            }

        for (const auto &[key, ix]: key__ix)
            if (ix)
                MSS(read_directory_(node.childs.nodes[*ix], directory/key, config));

        MSS_END();
    }
    bool read_directory(Node &node, const std::filesystem::path &directory, const Config &config)
    {
        MSS_BEGIN(bool);

        node.value.set(Type::Feature, directory.stem());
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

        const auto directory = parent_directory/item.key;

        const auto content_fp = is_leaf ? config.content_fp_leaf(directory) : config.content_fp_nonleaf(directory);
        std::filesystem::create_directories(content_fp.parent_path());

        std::string content;
        MSS(write_string(content, node));

        MSS(gubg::file::write(content, content_fp));

        for (const auto &child: node.childs.nodes)
        {
            if (!child.value.type)
                continue;
            switch (*child.value.type)
            {
                case Type::Requirement:
                case Type::Design:
                case Type::Feature:
                    if (!child.value.is_embedded() && !child.value.link)
                        MSS(write_directory(directory, child, config));
                    break;
                case Type::File:
                case Type::Directory:
                    continue;
                    break;
            }
        }

        MSS_END();
    }


    class Data
    {
    public:
        std::string title;
        std::vector<std::string> description;

        std::function<void()> process;

        ~Data() {clear();}

        bool valid() const {return !!style_;}
        operator bool () const {return valid();}

        std::optional<Style> style() const {return style_;}

        void clear()
        {
            if (valid())
            {
                if (process)
                    process();
                style_.reset();
            }
            title.clear();
            description.clear();
            style_.reset();
            add_newline_ = false;
        }
        void emplace(Style style)
        {
            clear();
            style_ = style;
        }

        void add_description(const std::string &str)
        {
            if (!valid())
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
        std::optional<Style> style_;
        bool add_newline_ = false;
    };

    bool read_string_(Node &node, const std::string &markdown)
    {
        MSS_BEGIN(bool);

        std::optional<Type> type;
        std::optional<Priority> priority;
        std::map<std::string, unsigned int> virtualkey__count;

        Data data;
        data.process = [&]()
        {
            if (!type)
            {
                //data is for node.value itself
                node.value.title = data.title;
                node.value.style = data.style();
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
                        child.value.set(*type, it->second);
                    }
                    else if (has("path"))
                    {
                        child.value.set(*type, it->second);
                        child.value.link = to_path(it->second);
                    }
                }
                else
                {
                    std::string virtualkey = data.title;
                    for (auto &ch: virtualkey)
                    {
                        ch = std::tolower(ch);
                        switch (ch)
                        {
                            case ' ': ch = '_'; break;
                            default: break;
                        }
                    }
                    const auto count = virtualkey__count[virtualkey]++;
                    if (count > 0)
                    {
                        virtualkey += "-";
                        virtualkey += std::to_string(count);
                    }
                    child.value.set(*type, std::string("#")+virtualkey);
                    child.value.title = data.title;
                    child.value.style = data.style();
                    child.value.description = data.description;
                    if (priority)
                        child.value.priority = *priority;
                }
            }
        };

        auto process_line = [&](const std::string &line, const std::string &prefix)
        {
            MSS_BEGIN(bool);

            if (prefix == "# ")
            {
                const bool is_first_title = !data;
                data.emplace(Style::Section);
                if (!is_first_title)
                    type = Type::Free;
                data.title = line;
            }
            else if (prefix == "===")
            {
                const bool is_first_title = !data;
                data.emplace(is_first_title ? Style::Title : Style::Section);
                if (!is_first_title)
                    type = Type::Free;
                data.title = line;
            }
            else if (prefix == "## ")
            {
                data.clear();

                if (false) {}
                else if (line == "Requirements") {type = Type::Requirement;}
                else if (line == "Design")       {type = Type::Design;}
                else if (line == "Features")     {type = Type::Feature;}
                else
                {
                    type = Type::Free;
                    data.emplace(Style::Section);
                    data.title = line;
                }
            }
            else if (!type)
            {
                if (data)
                    data.add_description(prefix+line);
            }
            else
            {
                //We know the item type

                switch (*type)
                {
                    case Type::Free:
                        data.add_description(prefix+line);
                        break;
                    case Type::Requirement:
                        if (prefix == "### ")
                        {
                            data.clear();
                            if (false) {}
                            else if (line == "Must")   {priority = Priority::Must;}
                            else if (line == "Should") {priority = Priority::Should;}
                            else if (line == "Could")  {priority = Priority::Could;}
                            else if (line == "Wont")   {priority = Priority::Wont;}
                            else
                            {
                                priority.reset();
                                data.emplace(Style::Section);
                                data.title = line;
                            }
                        }
                        else if (prefix == "* ")
                        {
                            data.emplace(Style::Bullet);
                            data.title = line;
                        }
                        else if (data.style() == Style::Bullet)
                        {
                            if (prefix == "  ")
                                data.add_description(line);
                        }
                        break;
                    case Type::Design:
                    case Type::Feature:
                        if (prefix == "### ")
                        {
                            data.emplace(Style::Section);
                            data.title = line;
                        }
                        else if (data.style() == Style::Section)
                        {
                            //As long as we're in a section: collect description
                            data.add_description(prefix+line);
                        }
                        else
                        {
                            if (prefix == "* ")
                            {
                                data.emplace(Style::Bullet);
                                data.title = line;
                            }
                            else if (data.style() == Style::Bullet)
                            {
                                if (prefix == "  ")
                                    data.add_description(line);
                                else if (line.empty())
                                {
                                    data.clear();
                                }
                                else if (!line.empty())
                                {
                                    data.emplace(Style::Margin);
                                    data.add_description(line);
                                }
                            }
                            else if (data.style() == Style::Margin)
                            {
                                if (line.empty())
                                    data.clear();
                                else
                                    data.add_description(line);
                            }
                            else if (line.empty())
                            {
                                data.clear();
                            }
                            else
                            {
                                data.emplace(Style::Margin);
                                data.add_description(line);
                            }
                        }
                        break;
                }
            }

            MSS_END();
        };

        std::optional<gubg::Strange> prev_line;
        auto process_prev_line = [&](const std::string &prefix)
        {
            MSS_BEGIN(bool);
            if (prev_line)
                MSS(process_line(prev_line->str(), prefix));
            prev_line.reset();
            MSS_END();
        };

        auto process_line_raw = [&](gubg::Strange &line)
        {
            MSS_BEGIN(bool);

            if (line.pop_if("# "))
            {
                MSS(process_prev_line(""));
                MSS(process_line(line.str(), "# "));
            }
            else if (line.pop_if("==="))
            {
                MSS(process_prev_line("==="));
            }
            else if (line.pop_if("## "))
            {
                MSS(process_prev_line(""));
                process_line(line.str(), "## ");
            }
            else if (line.pop_if("### "))
            {
                MSS(process_prev_line(""));
                MSS(process_line(line.str(), "### "));
            }
            else if (line.pop_if("* "))
            {
                MSS(process_prev_line(""));
                MSS(process_line(line.str(), "* "));
            }
            else if (line.pop_if("  "))
            {
                MSS(process_prev_line(""));
                MSS(process_line(line.str(), "  "));
            }
            else
            {
                MSS(process_prev_line(""));
                prev_line = line;
            }

            MSS_END();
        };

        //Parse the markdown document line-by-line
        gubg::Strange strange{markdown};
        for (gubg::Strange line; strange.pop_line(line); )
        {
            MSS(process_line_raw(line));
        }

        //Make sure the prev_line is processed as well
        {
            gubg::Strange line;
            MSS(process_line_raw(line));
        }


        MSS_END();
    }
    bool read_string(Node &node, const std::string &markdown)
    {
        MSS_BEGIN(bool);

        MSS(read_string_(node, markdown));

        const bool check_node_is_equivalent_with_original = true;
        if (check_node_is_equivalent_with_original)
        {
            std::string my_markdown;
            MSS(write_string(my_markdown, node));
            //TODO: Enable this again to ensure metadata is checked as well
            const bool check_metadata = false;
            if (!are_equivalent(markdown, my_markdown, check_metadata))
            {
                const std::filesystem::path original_fp = "/tmp/original.md";
                const std::filesystem::path my_fp = "/tmp/my.md";
                gubg::file::write(markdown, original_fp);
                gubg::file::write(my_markdown, my_fp);
                std::ostringstream oss;
                oss << "meld " << original_fp << " " << my_fp;
                std::system(oss.str().c_str());
                MSS(false);
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

        unsigned int section_level = 0;
        auto stream_section = [&](unsigned int level, const std::string &title, std::optional<Style> style = std::nullopt)
        {
            section_level = level;
            if (section_level > 1)
                oss << '\n';
            if (style.value_or(Style::Section) == Style::Section)
            {
                oss << std::string(section_level, '#') << ' ' << title << '\n';
            }
            else
            {
                oss << title << '\n';
                oss << std::string(title.size(), '=') << '\n';
            }

            new_section.reset();
        };

        {
            const auto title = node.value.title;
            if (!title.empty())
                stream_section(1, title, node.value.style);
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
            const auto child_type = child.value.type.value_or(Type::File);
            const auto child_prio = child.value.priority;

            bool skip_inclusion = false;
            switch (child_type)
            {
                case Type::Requirement:
                    if (child_type != current_section)
                    {
                        stream_section(2, "Requirements");
                        current_prio.reset();
                    }
                    if (child_prio != current_prio)
                    {
                        if (child_prio)
                            stream_section(3, hr(*child_prio));
                        else
                            section_level = 2;
                    }
                    current_prio = child_prio;
                    break;
                case Type::Design:
                    if (child_type != current_section)
                        stream_section(2, "Design");
                    break;
                case Type::Feature:
                    if (child_type != current_section)
                        stream_section(2, "Features");
                    break;
                case Type::Free:
                    break;
                case Type::File:
                case Type::Directory:
                    skip_inclusion = true;
                    break;
            }

            if (skip_inclusion)
                continue;

            current_section = child_type;

            if (child_type == Type::Free)
            {
                stream_section(2, child.value.title);
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
                    std::string indent;
                    bool reset_new_section_after_description = false;
                    switch (child.value.style.value_or(Style::Bullet))
                    {
                        case Style::Title:
                            oss << child.value.title << std::endl;
                            oss << std::string(child.value.title.size(), '=') << std::endl;
                            new_section.reset();
                            break;
                        case Style::Section:
                            oss << std::string(section_level+1, '#') << ' ' << child.value.title << std::endl;
                            new_section.reset();
                            break;
                        case Style::Bullet:
                            oss << "* " << child.value.title << std::endl;
                            indent = "  ";
                            break;
                        case Style::Margin:
                            if (!child.value.title.empty())
                            {
                                oss << child.value.title << std::endl;
                                new_section.reset();
                            }
                            reset_new_section_after_description = true;
                            break;
                    }
                    if (new_section())
                        oss << std::endl;
                    for (const auto &desc: child.value.description)
                        oss << indent << desc << std::endl;
                    if (reset_new_section_after_description)
                        new_section.reset();
                }
                else
                {
                    if (child.value.link)
                        oss << "* [external](path:" << to_string(*child.value.link) << ")\n";
                    else
                        oss << "* [external](key:" << child.value.key << ")\n";
                }
            }
        }

        markdown = oss.str();
        MSS_END();
    }

    bool are_equivalent(const std::string &m1, const std::string &m2, bool check_metadata)
    {
        auto forward = [check_metadata](auto &ix, const std::string &str)
        {
            const bool skip_metadata = true;

            if (!check_metadata)
                if (ix == 0)
                {
                    const std::string begin_marker = "<!--";
                    const std::string end_marker = "-->";
                    if (str.substr(ix, begin_marker.size()) == begin_marker)
                    {
                        const auto eix = str.find(end_marker, ix);
                        if (eix != std::string::npos)
                            ix = eix+end_marker.size();
                    }
                }

            for (; ix < str.size(); ++ix)
            {
                switch (str[ix])
                {
                    case '\n':
                    case '\r':
                    case '\t':
                    case ' ':
                    case '=':
                    case '#':
                        //These characters are skipped
                        break;

                    default:
                        return true;
                }
            }
            return false;
        };

        std::size_t ix1 = 0;
        std::size_t ix2 = 0;

        while (true)
        {
            const auto b1 = forward(ix1, m1);
            const auto b2 = forward(ix2, m2);

            if (b1 != b2)
                //One is finished, the other is not
                return false;

            if (!b1)
                //Both are finished and hence equal
                return true;

            //We can compare a next char
            if (m1[ix1] != m2[ix2])
                return false;

            ++ix1;
            ++ix2;
        }

        return false;
    }

} } } 
