#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <gubg/OnlyOnce.hpp>
#include <list>
#include <fstream>
#include <cassert>

namespace proast { namespace model { 

    bool Tree::find_root_filepath(std::filesystem::path &root, const std::filesystem::path &start)
    {
        MSS_BEGIN(bool);

        root = start;
        while (true)
        {
            if (std::filesystem::exists(root / "@root"))
                return true;
            const auto parent = root.parent_path();
            MSS_Q(parent != root);
            root = parent;
        }

        MSS_END();
    }

    bool Tree::load(const std::filesystem::path &root)
    {
        MSS_BEGIN(bool);
        root_filepath_.clear();
        root_forest_.nodes.resize(1);
        MSS(load_(root_forest_.nodes[0], root));
        root_filepath_ = root;
        MSS_END();
    }

    const std::filesystem::path &Tree::root_filepath() const { return root_filepath_; }

    Path Tree::root_path() const
    {
        Path path;
        if (!root_forest_.empty())
            path.emplace_back(root_forest_.nodes[0].value.short_name);
        return path;
    }

    bool Tree::find(const Forest *&forest, std::size_t &ix, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS(!path.empty());

        const Forest *my_forest = nullptr;
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

            //Search the my_ix for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.short_name == segment)
                    break;
            MSS(my_ix < my_forest->size());
        }

        forest = my_forest;
        ix = my_ix;

        MSS_END();
    }

    void Tree::stream(std::ostream &os) const
    {
        auto ftor = [](std::ostream &os, const Node &n)
        {
            os << n.value.short_name << std::endl;
        };
        gubg::tree::stream(os, root_forest_, ftor);
    }

    //Privates
    bool Tree::load_(Node &node, std::filesystem::path path)
    {
        MSS_BEGIN(bool);
        L(C(path));

        node.value.path = path;

        if (false) {}
        else if (std::filesystem::is_directory(path))
        {
            node.value.short_name = path.filename().string();
        }
        else if (std::filesystem::is_regular_file(path))
        {
            if (false) {}
            else
            {
                node.value.short_name = path.stem().string();

                {
                    std::ifstream fi{path};
                    gubg::OnlyOnce is_title;
                    for (std::string str; std::getline(fi, str); )
                    {
                        gubg::markup::Style style;
                        style.attention = (is_title() ? 1 : 0);
                        auto ftor = [&](auto &line) { line.add(str, style); };
                        node.value.preview.add_line(ftor);
                    }
                }

                //This is a file: there is no folder to recurse
                return true;
            }
        }
        else
        {
            MSS(false);
        }

        std::list<std::filesystem::path> sub_paths;
        for (const auto &sub: std::filesystem::directory_iterator{path})
        {
            const auto sub_path = sub.path();
            const auto fn_str = sub_path.filename().string();
            if (fn_str.empty())
                continue;
            switch (fn_str[0])
            {
                case '@':
                case '.':
                    break;
                default:
                    if (false) {}
                    else if (std::filesystem::is_directory(sub_path))
                    {
                        sub_paths.emplace_back(sub_path);
                    }
                    else if (std::filesystem::is_regular_file(sub_path))
                    {
                        const auto ext_str = sub_path.extension().string();
                        bool do_add = false;
                        if (ext_str == ".md" && sub_path.filename() != "readme.md")
                            do_add = true;
                        if (do_add)
                        {
                            sub_paths.emplace_back(sub_path);
                        }
                    }
                    break;
            }
        }

        node.childs.nodes.resize(sub_paths.size());
        auto n = node.childs.nodes.begin();
        for (const auto &sub_path: sub_paths)
        {
            MSS(load_(*n, sub_path));
            ++n;
        }

        MSS_END();
    }

} } 
