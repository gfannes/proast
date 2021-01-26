#include <proast/model/Model.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace model { 
    bool Model::add_root(const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        bool is_first = (tree.root.nr_childs() == 0);

        MSS(tree.add(path, config));

        if (is_first)
            current_path_ = {0u};

        MSS_END();
    }

    bool Model::move(Direction direction, int level)
    {
        MSS_BEGIN(bool);
        switch (direction)
        {
            case Direction::Down:
            case Direction::Up:
                if (level == 0)
                {
                    auto parent = current_parent();
                    MSS(!!parent);

                    MSS(!current_path_.empty());
                    {
                        auto &me_ix = current_path_.back();
                        switch (direction)
                        {
                            case Direction::Down:
                                MSS(me_ix < parent->nr_childs()-1);
                                ++me_ix;
                                break;
                            case Direction::Up:
                                MSS(me_ix > 0);
                                --me_ix;
                                break;
                        }
                    }

                    auto me = current_me();
                    MSS(!!me);
                    parent->value.selected = me->value.name;
                }
                else
                {
                    auto grand_parent = current_grand_parent();
                    MSS(!!grand_parent);

                    MSS(current_path_.size() >= 2);
                    {
                        auto &parent_ix = current_path_[current_path_.size()-2];
                        switch (direction)
                        {
                            case Direction::Down:
                                MSS(parent_ix < grand_parent->nr_childs()-1);
                                ++parent_ix;
                                break;
                            case Direction::Up:
                                MSS(parent_ix > 0);
                                --parent_ix;
                                break;
                        }
                    }

                    auto parent = current_parent();
                    MSS(!!parent);
                    grand_parent->value.selected = parent->value.name;
                }
                break;
            case Direction::Left:
                {
                    MSS(!current_path_.empty());
                    current_path_.pop_back();
                }
                break;
            case Direction::Right:
                if (tree.is_leaf(current_path_))
                {
                }
                else
                {
                    auto me = current_me();
                    MSS(!!me);
                    current_path_.emplace_back(Tree::selected_ix(*me));
                }
                break;
        }
        MSS_END();
    }

    Tree::Node *Model::current_me()
    {
        auto path = current_path_;
        Tree::Node *n = &tree.root;
        for (auto ix: path)
        {
            if (ix >= n->nr_childs())
                return nullptr;
            n = &n->childs.nodes[ix];
        }
        return n;
    }
    Tree::Node *Model::current_parent()
    {
        auto path = current_path_;
        if (path.empty())
            return nullptr;
        path.pop_back();
        Tree::Node *n = &tree.root;
        for (auto ix: path)
        {
            if (ix >= n->nr_childs())
                return nullptr;
            n = &n->childs.nodes[ix];
        }
        return n;
    }
    Tree::Node *Model::current_grand_parent()
    {
        auto path = current_path_;
        if (path.size() < 2)
            return nullptr;
        path.pop_back();
        path.pop_back();
        Tree::Node *n = &tree.root;
        for (auto ix: path)
        {
            if (ix >= n->nr_childs())
                return nullptr;
            n = &n->childs.nodes[ix];
        }
        return n;
    }
} } 
