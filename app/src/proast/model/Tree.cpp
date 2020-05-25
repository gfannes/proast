#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <gubg/OnlyOnce.hpp>
#include <map>
#include <fstream>
#include <cassert>

namespace proast { namespace model { 

    bool Tree::find_root_filepath(std::filesystem::path &root, const std::filesystem::path &start)
    {
        MSS_BEGIN(bool);

        root = start;
        while (true)
        {
            if (std::filesystem::exists(root / ".proast_root"))
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
        MSS(load_(root_forest_.nodes[0], root.stem().string(), root));
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

    bool Tree::find(Forest *&forest, std::size_t &ix, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS(!path.empty());

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
                if (my_forest->nodes[my_ix].value.short_name == segment)
                    break;
            MSS(my_ix < my_forest->size());
        }

        forest = my_forest;
        ix = my_ix;

        MSS_END();
    }
    bool Tree::find(Node *&node, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS(!path.empty());

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
                if (my_forest->nodes[my_ix].value.short_name == segment)
                    break;
            MSS(my_ix < my_forest->size());
        }

        node = &my_forest->nodes[my_ix];

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
    bool Tree::load_(Node &node, const std::string &stem, std::filesystem::path path)
    {
        MSS_BEGIN(bool);
        L(C(path));

        node.value.short_name = stem;
        node.value.path = path;

        auto load_preview = [&]()
        {
            std::ifstream fi{node.value.path};
            gubg::OnlyOnce is_title;
            for (std::string str; std::getline(fi, str); )
            {
                gubg::markup::Style style;
                style.attention = (is_title() ? 1 : 0);
                auto ftor = [&](auto &line) { line.add(str, style); };
                node.value.preview.add_line(ftor);
            }
        };


        if (false) {}
        else if (std::filesystem::is_directory(path))
        {
            auto check_for_content = [&](const auto &content_path)
            {
                if (std::filesystem::is_regular_file(content_path))
                {
                    node.value.path = content_path;

                    load_preview();
                    return true;
                }
                return false;
            };
            check_for_content(path / "readme.md") || check_for_content(std::filesystem::path{path} += ".md");
        }
        else if (std::filesystem::is_regular_file(path))
        {
            load_preview();

            //This is a file: there is no folder to recurse
            return true;
        }
        else
        {
            MSS(false);
        }

        using Stem__Path = std::map<std::string, std::filesystem::path>;
        Stem__Path stem__dir;
        Stem__Path stem__file;
        for (const auto &sub: std::filesystem::directory_iterator{path})
        {
            const auto sub_path = sub.path();

            const auto stem = sub_path.stem().string();
            if (stem.empty())
                continue;
            switch (stem[0])
            {
                case '.':
                    break;
                default:
                    if (false) {}
                    else if (std::filesystem::is_directory(sub_path))
                    {
                        stem__dir.emplace(stem, sub_path);
                    }
                    else if (std::filesystem::is_regular_file(sub_path))
                    {
                        bool do_add = false;
                        {
                            const auto ext_str = sub_path.extension().string();
                            if (ext_str == ".md" && sub_path.filename() != "readme.md")
                                do_add = true;
                        }
                        if (do_add)
                            stem__file.emplace(stem, sub_path);
                    }
                    break;
            }
        }

        Stem__Path stem__path;
        {
            stem__path.swap(stem__dir);

            for (const auto &[stem, file]: stem__file)
                if (!stem__path.count(stem))
                    stem__path.emplace(stem, file);
        }

        node.childs.nodes.resize(stem__path.size());
        auto n = node.childs.nodes.begin();
        for (const auto &[stem, subpath]: stem__path)
        {
            MSS(load_(*n, stem, subpath));
            ++n;
        }

        MSS_END();
    }

} } 
