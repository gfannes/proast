#include <proast/model/Model.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace model { 
    Model::Model()
    {
        node_ = &tree.root;
    }

    bool Model::add_root(const std::filesystem::path &path, const Tree::Config &config)
    {
        return tree.add(path, config);
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
        return node_;
    }
    Node *Model::node_00()
    {
        return node_->value.navigation.child;
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
        auto s = log::Scope("Model.move()", [&](auto &h){h.attr("me", me);});
        MSS_BEGIN(bool);
        switch (direction)
        {
            case Direction::Down:
                if (me)
                {
                    if (auto child = node_->value.navigation.child)
                        if (auto &childchild = child->value.navigation.child)
                            if (auto down = childchild->value.navigation.down)
                                childchild = down;
                }
                else
                {
                    if (auto &child = node_->value.navigation.child)
                        if (auto down = child->value.navigation.down)
                        {
                            child = down;
                            s.line([&](auto &os){os << "new child " << child << " for " << node_;});
                        }
                    s.line([](auto &os){os << "ola";});
                }
                break;
            case Direction::Up:
                if (me)
                {
                    if (auto child = node_->value.navigation.child)
                        if (auto &childchild = child->value.navigation.child)
                            if (auto up = childchild->value.navigation.up)
                                childchild = up;
                }
                else
                {
                    if (auto &child = node_->value.navigation.child)
                        if (auto up = child->value.navigation.up)
                            child = up;
                }
                break;
            case Direction::Left:
                if (node_->value.navigation.parent)
                    node_ = node_->value.navigation.parent;
                break;
            case Direction::Right:
                if (node_->value.navigation.child)
                    node_ = node_->value.navigation.child;
                break;
        }
        s.line([](auto &os){os << "olb";});
        MSS_END();
    }
} } 
