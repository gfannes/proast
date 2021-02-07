#include <proast/model/Tree.hpp>
#include <proast/util.hpp>
#include <proast/log.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

namespace proast { namespace model { 
    Tree::Tree()
    {
        root.value.name = "<ROOT>";
        root2 = Node_::create("<ROOT>");
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

        MSS(std::filesystem::is_directory(path), log::ostream() << "Cannot add " << path << ", this is not a directory" << std::endl);

        auto child2 = root2->append_child();
        //TODO: allow base node to have a different name than path.filename()
        child2->segment = path;

        MSS(add_(child2, path, config));
        log::raw([&](auto &os){os << "Loaded " << child2->node_count() << " nodes from \"" << path << "\"" << std::endl;});

        auto &child = root.childs.append();
        child.value.name = path.filename().string();
        child.value.path = path;

        MSS(add_(child, path, config));
        log::raw([&](auto &os){os << "Loaded " << child.node_count() << " nodes from \"" << path << "\"" << std::endl;});

        compute_navigation_(root);

        if (auto fn = metadata_fn_(child.value.path); !std::filesystem::is_regular_file(fn))
            log::raw([&](auto &os){os << "Warning: could not load metadata from " << fn << ", this file does not exist." << std::endl;});
        else if (append_metadata_(fn))
            set_metadata_();

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

    void Tree::recompute_metadata(Node &node)
    {
        auto &my_md = node.value.metadata;
        my_md.dependencies.clear();
        for (auto &child: node.childs.nodes)
        {
            //Depth-first search
            recompute_metadata(child);

            node.value.metadata.dependencies.add(&child);
            node.value.metadata.dependencies.add(child.value.metadata.dependencies);
        }
    }
    bool Tree::stream_metadata()
    {
        MSS_BEGIN(bool);

        for (auto &child: root.childs.nodes)
        {
            std::ofstream fo{metadata_fn_(child.value.path)};
            MSS(stream_metadata_(fo, child));
        }

        MSS_END();
    }

    //Privates
    bool Tree::stream_metadata_(std::ostream &os, Node &node)
    {
        MSS_BEGIN(bool);
        gubg::naft::Document doc{os};
        if (node.value.metadata.has_local_data())
        {
            auto n = doc.node("Metadata");
            n.attr("path", to_string(to_path(&node)));
            node.value.metadata.stream(n);
        }
        for (auto &child: node.childs.nodes)
        {
            //Depth-first search
            MSS(stream_metadata_(os, child));
        }
        MSS_END();
    }
    bool Tree::append_metadata_(const std::filesystem::path &fp)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, fp));
        gubg::naft::Range range{content};

        std::string key, value;
        while (range.pop_tag("Metadata"))
        {
            MSS(range.pop_attr(key, value));
            MSS(key == "path");
            const auto path = to_path(value);
            auto &md = path__metadata_[path];
            gubg::naft::Range subrange;
            MSS(range.pop_block(subrange));
            MSS(md.parse(subrange));
        }

        MSS_END();
    }
    void Tree::set_metadata_()
    {
        for (const auto &[path,md]: path__metadata_)
            if (auto n = find(path))
                n->value.metadata.set_when_unset(md);
    }

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
            child.value.name = path.filename().string();
            if (is_folder)
                MSS(add_(child, path, config));
        }

        MSS_END();
    }
    bool Tree::add_(Node2 node, const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        MSS(!!node);

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
            auto child = node->append_child();
            child->segment = path.filename().string();
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

    std::filesystem::path Tree::metadata_fn_(const std::filesystem::path &base_dir)
    {
        return base_dir / "proast-metadata.naft";
    }
} } 
