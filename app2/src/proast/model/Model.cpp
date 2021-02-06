#include <proast/model/Model.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <filesystem>

namespace proast { namespace model { 
    Model::Model()
    {
        current_node_ = &tree_.root;
    }
    Model::~Model()
    {
        if (!bookmarks_.save(bookmarks_fp_))
            log::ostream() << "Warning: could not save bookmarks to " << bookmarks_fp_ << std::endl;
    }

    bool Model::set_home(const std::filesystem::path &home_dir)
    {
        MSS_BEGIN(bool);

        if (!std::filesystem::is_directory(home_dir))
            std::filesystem::create_directories(home_dir);
        MSS(std::filesystem::is_directory(home_dir));

        home_dir_ = home_dir;

        {
            bookmarks_fp_ = home_dir/"bookmarks.naft";
            if (!bookmarks_.load(bookmarks_fp_))
                log::ostream() << "Warning: could not load bookmarks from " << bookmarks_fp_ << std::endl;
        }

        MSS_END();
    }

    bool Model::add_root(const std::filesystem::path &path, const Tree::Config &config)
    {
        MSS_BEGIN(bool);

        root_config_ary_.emplace_back(path, config);

        MSS(tree_.add(path, config));

        tree_.recompute_metadata(tree_.root);

        MSS_END();
    }

    bool Model::reload()
    {
        MSS_BEGIN(bool);

        const auto restore_path = to_path(node());

        bookmarks_.save(bookmarks_fp_);

        MSS(set_home(home_dir_));

        tree_.clear();
        //Copy root_config_ary_ because it will be updated during calls to add_root()
        auto root_config_ary_copy = root_config_ary_;
        root_config_ary_.clear();
        for (const auto &[path, config]: root_config_ary_copy)
        {
            log::ostream() << path << std::endl;
            MSS(add_root(path, config));
        }

        focus(restore_path);

        MSS_END();
    }

    bool Model::focus(const Path &path)
    {
        MSS_BEGIN(bool);

        auto node = tree_.find(path);
        MSS(!!node);
        //Reroute navigation.child for parent and grand_parent
        for (auto ix = 0u; ix < 2 && node; ++ix)
            if (auto parent = node->value.navigation.parent)
            {
                parent->value.navigation.child = node;
                node = parent;
            }
        current_node_ = node;

        MSS_END();
    }

    bool Model::register_bookmark(wchar_t wchar)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        bookmarks_.set(wchar, to_path(n));

        MSS_END();
    }
    bool Model::jump_to_bookmark(wchar_t wchar)
    {
        MSS_BEGIN(bool);

        Path path;
        MSS(bookmarks_.get(path, wchar));

        MSS(focus(path));

        MSS_END();
    }

    Node *Model::node()
    {
        if (auto n = node_000())
            return n;
        if (auto n = node_00())
            return n;
        if (auto n = node_0())
            return n;
        return nullptr;
    }
    Node *Model::node_0()
    {
        return current_node_;
    }
    Node *Model::node_00()
    {
        return current_node_->value.navigation.child;
    }
    Node *Model::node_0a()
    {
        auto node00 = node_00();
        if (node00)
            return node00->value.navigation.up;
        return nullptr;
    }
    Node *Model::node_0b()
    {
        auto node00 = node_00();
        if (node00)
            return node00->value.navigation.down;
        return nullptr;
    }
    Node *Model::node_000()
    {
        auto node00 = node_00();
        if (node00)
            return node00->value.navigation.child;
        return nullptr;
    }
    Node *Model::node_00a()
    {
        auto node000 = node_000();
        if (node000)
            return node000->value.navigation.up;
        return nullptr;
    }
    Node *Model::node_00b()
    {
        auto node000 = node_000();
        if (node000)
            return node000->value.navigation.down;
        return nullptr;
    }

    bool Model::move(Direction direction, bool me)
    {
        MSS_BEGIN(bool);
        switch (direction)
        {
            case Direction::Down:
                if (me)
                {
                    if (auto child = current_node_->value.navigation.child)
                        if (auto &childchild = child->value.navigation.child)
                        {
                            if (auto down = childchild->value.navigation.down)
                                childchild = down;
                        }
                        else if (auto content = child->value.content)
                        {
                            if (content->ix+1 < content->items.size())
                                ++content->ix;
                        }
                }
                else
                {
                    if (auto &child = current_node_->value.navigation.child)
                    {
                        if (auto down = child->value.navigation.down)
                        {
                            child = down;
                        }
                    }
                    else if (auto content = current_node_->value.content)
                    {
                        if (content->ix+1 < content->items.size())
                            ++content->ix;
                    }
                }
                break;
            case Direction::Up:
                if (me)
                {
                    if (auto child = current_node_->value.navigation.child)
                        if (auto &childchild = child->value.navigation.child)
                        {
                            if (auto up = childchild->value.navigation.up)
                                childchild = up;
                        }
                        else if (auto content = child->value.content)
                        {
                            if (content->ix > 0)
                                --content->ix;
                        }
                }
                else
                {
                    if (auto &child = current_node_->value.navigation.child)
                    {
                        if (auto up = child->value.navigation.up)
                            child = up;
                    }
                    else if (auto content = current_node_->value.content)
                    {
                        if (content->ix > 0)
                            --content->ix;
                    }
                }
                break;
            case Direction::Left:
                if (current_node_->value.navigation.parent)
                    current_node_ = current_node_->value.navigation.parent;
                break;
            case Direction::Right:
                if (current_node_->value.navigation.child)
                    current_node_ = current_node_->value.navigation.child;
                break;
        }
        MSS_END();
    }

    void Model::recompute_metadata()
    {
        tree_.recompute_metadata(tree_.root);
    }
    bool Model::sync_metadata()
    {
        return tree_.stream_metadata();
    }

} } 
