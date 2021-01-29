#include <proast/model/Tree.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <iostream>
#include <map>

namespace proast { namespace model { 
    Tree::Tree()
    {
        root.value.name = L"<ROOT>";
    }

    Tree::Config::Config()
    {
        for (const auto &fn: {"extern"})
            names_to_skip.insert(fn);
        for (const auto &ext: {".resp", ".a", ".obj", ".o", ".lib", ".dll", ".ut", ".app", ".exe", ".ninja"})
            extensions_to_skip.insert(ext);
    }

    bool Tree::add(const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        auto &child = root.childs.append();
        child.value.name = path.filename().wstring();
        child.value.path = path;

        MSS(add_(child, path, config));
        log::raw([&](auto &os){os << "Loaded " << child.node_count() << " nodes from \"" << path << "\"" << std::endl;});

        compute_navigation_(root);

        MSS_END();
    }

    Node *Tree::find(const Path &path)
    {
        Node *node = &root;
        for (const auto &part: path)
        {
            auto &childs = node->childs.nodes;
            node = nullptr;
            for (auto &child: childs)
                if (child.value.name == part)
                {
                    node = &child;
                    break;
                }
            if (!node)
                break;
        }
        return node;
    }

    std::size_t Tree::selected_ix(const Node &node)
    {
        const auto &child_nodes = node.childs.nodes;
        for (auto ix = 0u; ix < child_nodes.size(); ++ix)
            if (node.value.navigation.child == &child_nodes[ix])
                return ix;
        return 0;
    }

    //Privates
    bool Tree::add_(Node &node, const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        std::map<std::filesystem::path, bool> path__is_folder;

        for (auto &entry: std::filesystem::directory_iterator(path))
        {
            const auto path = entry.path();
            const auto fn = path.filename().string();
            const auto ext = path.extension();
            const auto is_hidden = fn.empty() ? true : fn[0]=='.';
            if (is_hidden || config.names_to_skip.count(fn) || config.extensions_to_skip.count(ext))
            {
            }
            else if (std::filesystem::is_regular_file(path))
            {
                path__is_folder[path] = false;
            }
            else if (std::filesystem::is_directory(path))
            {
                path__is_folder[path] = true;
            }
        }

        for (const auto &[path, is_folder]: path__is_folder)
        {
            auto &child = node.childs.append();
            child.value.path = path;
            child.value.name = path.filename().wstring();
            if (is_folder)
                MSS(add_(child, path, config));
        }

        MSS_END();
    }

    void Tree::compute_navigation_(Node &node)
    {
        Node *prev = nullptr;
        for (auto &child: node.childs.nodes)
        {
            if (!prev)
                //TODO: this default navigation setup should be updated with the saved navigation state
                node.value.navigation.child = &child;

            child.value.navigation.parent = &node;
            child.value.navigation.child = nullptr;
            child.value.navigation.up = prev;
            child.value.navigation.down = nullptr;
            if (prev)
                prev->value.navigation.down = &child;

            compute_navigation_(child);

            prev = &child;
        }
    }
} } 
