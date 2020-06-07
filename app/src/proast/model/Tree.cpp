#include <proast/model/Tree.hpp>
#include <proast/model/Markdown.hpp>
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

        //TODO: not sure why we first clear() only set it later
        name__root_filepath_[name].clear();

        auto &node = root_forest_.append();
        MSS(markdown::read_directory(node, root, cfg));

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
    bool Tree::find(NodeIXPath &nixpath, const Path &path)
    {
        MSS_BEGIN(bool);

        const auto depth = path.size();

        nixpath.resize(depth);
        nixpath.resize(0);

        MSS_Q(depth > 0);

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

            nixpath.emplace_back(&my_forest->nodes[my_ix], my_ix);
        }

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

} } 
