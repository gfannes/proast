#include <proast/model/Tree.hpp>
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

        MSS(add_(child, path, config));
        std::cout << "Loaded " << child.node_count() << " nodes from \"" << path << "\"" << std::endl;

        MSS_END();
    }

    bool Tree::resolve_datas(Datas &datas, const Path &path)
    {
        MSS_BEGIN(bool);

        datas.resize(path.size()+1);
        datas.resize(0u);

        Node *node = &root;
        datas.emplace_back(&node->value);
        for (auto ix: path)
        {
            MSS(ix < node->childs.size());
            node = &node->childs.nodes[ix];
            datas.emplace_back(&node->value);
        }

        MSS_END();
    }

    bool Tree::resolve_nodes(Nodes &nodes, const Path &path)
    {
        MSS_BEGIN(bool);

        nodes.resize(path.size()+1);
        nodes.resize(0u);

        Node *node = &root;
        nodes.emplace_back(node);
        for (auto ix: path)
        {
            MSS(ix < node->childs.size());
            node = &node->childs.nodes[ix];
            nodes.emplace_back(node);
        }

        MSS_END();
    }

    std::size_t Tree::selected_ix(const Node &node)
    {
        const auto &selected = node.value.selected;
        const auto &child_nodes = node.childs.nodes;
        for (auto ix = 0u; ix < child_nodes.size(); ++ix)
        {
            if (child_nodes[ix].value.name == selected)
                return ix;
        }
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
            std::cout << path << std::endl;
            if (is_folder)
                MSS(add_(child, path, config));
        }

        MSS_END();
    }
} } 
