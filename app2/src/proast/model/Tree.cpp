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

    void Tree::resolve_datas(Datas &datas, const Path &path)
    {
        datas.resize(path.size()+1);
        datas.resize(0u);

        Node *node = &root;
        datas.emplace_back(&node->value);
        for (auto ix: path)
        {
            if (node)
                if (0 <= ix && ix < node->nr_childs())
                    node = &node->childs.nodes[ix];
                else
                    node = nullptr;
            datas.emplace_back(node ? &node->value : nullptr);
        }
    }

    void Tree::resolve_nodes(Nodes &nodes, const Path &path)
    {
        nodes.resize(path.size()+1);
        nodes.resize(0u);

        Node *node = &root;
        nodes.emplace_back(node);
        for (auto ix: path)
        {
            if (node)
                if (0 <= ix && ix < node->nr_childs())
                    node = &node->childs.nodes[ix];
                else
                    node = nullptr;
            nodes.emplace_back(node);
        }
    }

    bool Tree::is_leaf(const Path &path) const
    {
        const Node *node = &root;
        for (auto ix: path)
        {
            if (ix < 0 || ix >= node->nr_childs())
                return false;
            node = &node->childs.nodes[ix];
        }
        return node->nr_childs() == 0;
    }

    std::size_t Tree::selected_ix(const Node &node)
    {
        const auto &child_nodes = node.childs.nodes;
#if 0
        const auto &selected = node.value.selected;
        //TODO: this linear search can be optimized
        for (auto ix = 0u; ix < child_nodes.size(); ++ix)
        {
            if (child_nodes[ix].value.name == selected)
                return ix;
        }
#else
        for (auto ix = 0u; ix < child_nodes.size(); ++ix)
            if (node.value.navigation.child == &child_nodes[ix])
                return ix;
#endif
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
