#include <proast/model/Tree.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/naft/Range.hpp>
#include <map>
#include <list>
#include <fstream>
#include <sstream>
#include <cassert>

namespace proast { namespace model { 

    bool Tree::load(const std::filesystem::path &root, const Config &cfg)
    {
        MSS_BEGIN(bool);

        const std::string name = root.stem().string();
        auto it = name__cfg_.find(name);
        MSS(it == name__cfg_.end());

        name__cfg_[name] = cfg;
        name__root_filepath_[name].clear();

        auto &node = root_forest_.append();
        MSS(load_(cfg, node, root.stem().string(), root));

        name__root_filepath_[name] = root;

        MSS_END();
    }

    std::filesystem::path Tree::root_filepath(const std::string &name) const
    {
        std::filesystem::path fp;
        auto it = name__root_filepath_.find(name);
        if (it != name__root_filepath_.end())
            fp = it->second;
        return fp;
    }

    Path Tree::first_root_path() const
    {
        Path path;
        if (!root_forest_.empty())
            path.emplace_back(root_forest_.nodes[0].value.key);
        return path;
    }

    bool Tree::find(Forest *&forest, std::size_t &ix, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS_Q(!path.empty());

        Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());
        }

        forest = my_forest;
        ix = my_ix;

        MSS_END();
    }
    bool Tree::find(Node *&node, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS_Q(!path.empty());

        Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());
        }

        node = &my_forest->nodes[my_ix];

        MSS_END();
    }

    void Tree::stream(std::ostream &os) const
    {
        auto ftor = [](std::ostream &os, const Node &n)
        {
            os << n.value.key << std::endl;
        };
        gubg::tree::stream(os, root_forest_, ftor);
    }

    //Privates
    bool Tree::load_(const Config &cfg, Node &node, const std::string &stem, const std::filesystem::path directory) const
    {
        MSS_BEGIN(bool);

        node.value.key = stem;
        node.value.directory = directory;

        std::list<Path> links;
        auto setup_content = [&](const std::filesystem::path &content_fp)
        {
            MSS_BEGIN(bool);
            node.value.content_fp = content_fp;

            std::ifstream fi{content_fp};
            gubg::OnlyOnce is_title;
            bool in_comment = false;
            std::ostringstream oss;
            for (std::string str; std::getline(fi, str); )
            {
                auto starts_with = [&](const std::string &needle)
                {
                    return needle == str.substr(0, needle.size());
                };
                if (in_comment)
                {
                    if (starts_with("-->"))
                    {
                        in_comment = false;

                        gubg::naft::Range range0{oss.str()};
                        if (range0.pop_tag("proast"))
                        {
                            for (const auto &[k,v]: range0.pop_attrs())
                            {
                                if (false) {}
                                else if (k == "my_cost") {node.value.my_cost = std::stod(v);}
                            }

                            gubg::naft::Range range1;
                            if (range0.pop_block(range1))
                            {
                                while (!range1.empty())
                                {
                                    if (false) {}
                                    else if (range1.pop_tag("link"))
                                    {
                                        for (const auto &[k,v]: range1.pop_attrs())
                                        {
                                            if (false) {}
                                            else if (k == "path") {links.push_back(to_path(v));}
                                            else
                                            {
                                                MSS(false, log::stream() << "Error: Unknown argument " << k << " for link" << std::endl);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        MSS(false, log::stream() << "Error: Could not parse metadata from " << content_fp << std::endl);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        oss << str << '\n';
                    }
                }
                else
                {
                    if (starts_with("<!--"))
                    {
                        in_comment = true;

                        oss.str("");
                    }
                    else
                    {
                        gubg::markup::Style style;
                        style.attention = (is_title() ? 1 : 0);
                        auto ftor = [&](auto &line) { line.add(str, style); };
                        node.value.preview.add_line(ftor);
                    }
                }
            }
            MSS_END();
        };
        auto add_links = [&]()
        {
            auto ix = node.childs.nodes.size();
            node.childs.nodes.resize(ix+links.size());
            for (auto link = links.begin(); ix < node.childs.nodes.size(); ++ix, ++link)
            {
                auto &link_node = node.childs.nodes[ix];
                link_node.value.link = *link;
                link_node.value.key = to_string(*link);
            }
        };

        const auto content_fp_leaf = cfg.content_fp_leaf(directory);
        const auto content_fp_nonleaf = cfg.content_fp_nonleaf(directory);

        if (!std::filesystem::exists(directory))
        {
            MSS(std::filesystem::is_regular_file(content_fp_leaf), log::stream() << "Error: directory " << directory << " does not exist, and neither does " << content_fp_leaf << std::endl);
            MSS(setup_content(content_fp_leaf));
            add_links();

            //This is a leaf: there is no folder to recurse
            return true;
        }

        //This should be directory, and hence a nonleaf
        MSS(std::filesystem::is_directory(directory));

        if (std::filesystem::exists(content_fp_leaf))
            log::stream() << "Warning: Leaf-type content files are ignored when the corresponding directory is present: " << content_fp_leaf << " " << directory << std::endl;

        if (std::filesystem::is_regular_file(content_fp_nonleaf))
            MSS(setup_content(content_fp_nonleaf));

        using Stem__Path = std::map<std::string, std::filesystem::path>;
        Stem__Path stem__dir;
        for (const auto &sub: std::filesystem::directory_iterator{directory})
        {
            const auto sub_path = sub.path();

            const auto stem = sub_path.stem().string();
            if (stem.empty() || stem == cfg.index_name())
                continue;

            switch (stem[0])
            {
                case '.':
                    break;
                default:
                    {
                        bool do_add = false;
                        if (false) {}
                        else if (std::filesystem::is_directory(sub_path))
                            do_add = true;
                        else if (std::filesystem::is_regular_file(sub_path))
                        {
                            const auto ext_str = sub_path.extension().string();
                            if (ext_str == cfg.extension())
                                do_add = true;
                        }

                        if (do_add)
                            stem__dir.emplace(stem, directory / stem);
                    }
                    break;
            }
        }

        node.childs.nodes.resize(stem__dir.size());
        auto n = node.childs.nodes.begin();
        for (const auto &[stem, dir]: stem__dir)
        {
            MSS(load_(cfg, *n, stem, dir));
            ++n;
        }

        add_links();

        MSS_END();
    }

} } 
