#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <list>

namespace proast { namespace model { 

    bool Tree::find_root(std::filesystem::path &root, const std::filesystem::path &start)
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
        root_path_.clear();
        root_forest_.nodes.resize(1);
        MSS(load_(root_forest_.nodes[0], root));
        root_path_ = root;
        MSS_END();
    }

    const std::filesystem::path &Tree::root_path() const { return root_path_; }

    bool Tree::find(const Forest *&forest, std::size_t &ix, const Path &path)
    {
        MSS_BEGIN(bool);

        forest = nullptr;
        ix = 0;

        if (!root_forest_.empty())
            forest = &root_forest_;

        auto follow_path = [&](const std::string &segment)
        {
            if (!forest)
                return;

            forest = &forest->nodes[ix].childs;

            const auto nr_childs = forest->size();
            for (ix = 0; ix < nr_childs; ++ix)
                if (forest->nodes[ix].value.short_name == segment)
                    break;

            if (ix >= nr_childs)
                forest = nullptr;
        };
        path.each_segment(follow_path);

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
                //This is a file: there is no folder to recurse
                return true;
            }
        }
        else
        {
            MSS(false);
        }

        std::cout << path << std::endl;
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
                        std::cout << " d " << sub_path << std::endl;
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
                            std::cout << " f " << sub_path << std::endl;
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
