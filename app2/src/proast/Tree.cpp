#include <proast/Tree.hpp>
#include <gubg/mss.hpp>
#include <iostream>
#include <map>

namespace proast { 
    Tree::Config::Config()
    {
        skip.insert(".git");
    }

    bool Tree::add(const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);
        MSS(add_(forest_, path, config));
        std::cout << "Loaded " << forest_.node_count() << " nodes from \"" << path << "\"" << std::endl;
        MSS_END();
    }

    bool Tree::add_(Forest &forest, const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        std::map<std::filesystem::path, bool> path__is_folder;

        for (auto &entry: std::filesystem::directory_iterator(path))
        {
            const auto path = entry.path();
            const auto fn = path.filename().string();
            const auto is_hidden = fn.empty() ? true : fn[0]=='.';
            if (is_hidden || config.skip.count(fn))
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
            forest.nodes.emplace_back();
            auto &node = forest.nodes.back();
            node.value.path = path;
            std::cout << path << std::endl;
            if (is_folder)
                MSS(add_(node.childs, path, config));
        }

        MSS_END();
    }
} 
