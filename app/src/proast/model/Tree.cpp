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
        MSS(load_(root_, root));
        root_path_ = root;
        MSS_END();
    }

    const std::filesystem::path &Tree::root_path() const { return root_path_; }

    const Node *Tree::find(const Path &path)
    {
        const Node *node = &root_;
        auto follow_path = [&](const std::string &segment)
        {
            if (!node)
                return;
            const Node *child_node_ptr = nullptr;
            for (const auto &child_node: node->childs.nodes)
            {
                if (child_node.value.short_name == segment)
                {
                    child_node_ptr = &child_node;
                    break;
                }
            }
            node = child_node_ptr;
        };
        path.each_segment(follow_path);
        return node;
    }

    void Tree::stream(std::ostream &os) const
    {
        auto ftor = [](std::ostream &os, const Node &n)
        {
            os << n.value.short_name << std::endl;
        };
        gubg::tree::stream(os, root_.childs, ftor);
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
