#include <proast/model/Model.hpp>
#include <proast/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/mss.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <cassert>

namespace proast { namespace model { 
    Model::Model()
    {
        root_ = Node_::create("<ROOT>");
        current_node_ = root_;
    }
    Model::~Model()
    {
        if (!bookmarks_.save(bookmarks_fp_))
            log::ostream() << "Warning: could not save bookmarks to " << bookmarks_fp_ << std::endl;
        if (!save_current_location_())
            log::ostream() << "Warning: could not save the current location to " << current_location_fn_ << std::endl;
    }

    Model::Config::Config()
    {
        for (const auto &fn: {"extern"})
            names_to_skip.insert(fn);
        for (const auto &ext: {".resp", ".a", ".obj", ".o", ".lib", ".dll", ".ut", ".app", ".exe", ".ninja"})
            extensions_to_skip.insert(ext);
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
        current_location_fn_ = home_dir/"current_location.txt";

        MSS_END();
    }

    bool Model::add_root(const std::filesystem::path &path, const Config &config)
    {
        MSS_BEGIN(bool);

        MSS(std::filesystem::is_directory(path), log::ostream() << "Cannot add " << path << ", this is not a directory" << std::endl);

        root_config_ary_.emplace_back(path, config);

        MSS(!!root_);
        auto child = root_->append_child();
        //TODO: allow base node to have a different name than path.filename()
        child->segment = path;

        MSS(add_(child, path, config));
        log::raw([&](auto &os){os << "Loaded " << child->node_count() << " nodes from \"" << path << "\"" << std::endl;});

        compute_navigation_(root_);

        if (auto fn = metadata_fn_(child->path()); !std::filesystem::is_regular_file(fn))
            log::raw([&](auto &os){os << "Warning: could not load metadata from " << fn << ", this file does not exist." << std::endl;});
        else if (append_metadata_(fn))
            set_metadata_();

        recompute_metadata();

        //TODO: rework into adding all roots at once so we know when we can set the current location
        load_current_location_();

        MSS_END();
    }

    bool Model::reload()
    {
        MSS_BEGIN(bool);

        save_current_location_();
        bookmarks_.save(bookmarks_fp_);

        MSS(set_home(home_dir_));

        depth_first_search(root_, [](auto &n){n->clear_dependencies();});
        path__metadata_.clear();
        root_ = Node_::create("<ROOT>");

        //Copy root_config_ary_ because it will be updated during calls to add_root()
        auto root_config_ary_copy = root_config_ary_;
        root_config_ary_.clear();
        for (const auto &[path, config]: root_config_ary_copy)
        {
            log::ostream() << path << std::endl;
            MSS(add_root(path, config));
        }

        load_current_location_();

        MSS_END();
    }

    bool Model::focus(const Path &path)
    {
        MSS_BEGIN(bool);

        MSS(!!root_);

        if (auto node = root_->find(path))
        {
            //Reroute navigation.child for parent and grand_parent
            for (auto ix = 0u; ix < 2 && node; ++ix)
                if (auto parent = node->parent.lock())
                {
                    parent->child = node;
                    node = parent;
                }
            current_node_ = node;
        }

        MSS_END();
    }

    bool Model::create(const std::string &name, bool create_file, bool create_in)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        if (create_in)
            MSS(create_(n, name, create_file));
        else
        {
            auto parent = n->parent.lock();
            MSS(!!parent);
            MSS(create_(parent, name, create_file));
        }

        MSS_END();
    }
    bool Model::rename(const std::string &name)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        const auto orig_fp = n->path();
        n->segment = name;
        const auto new_fp = n->path();

        std::filesystem::rename(orig_fp, new_fp);

        MSS_END();
    }
    bool Model::delete_current()
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        //TODO: move to scratchpad iso actually deleting
        const auto path = n->path();
        if (std::filesystem::is_regular_file(path))
            std::filesystem::remove(path);
        else
            std::filesystem::remove_all(path);

        MSS(reload());

        MSS_END();
    }


    bool Model::register_bookmark(char ch)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        bookmarks_.set(ch, n->to_path());

        MSS_END();
    }
    bool Model::jump_to_bookmark(char ch)
    {
        MSS_BEGIN(bool);

        Path path;
        MSS(bookmarks_.get(path, ch));

        MSS(focus(path));

        MSS_END();
    }

    Node Model::node()
    {
        if (auto n = node_000())
            return n;
        if (auto n = node_00())
            return n;
        if (auto n = node_0())
            return n;
        return Node{};
    }
    Node Model::node_0()
    {
        return current_node_;
    }
    Node Model::node_00()
    {
        if (current_node_)
            return current_node_->child.lock();
        return Node{};
    }
    Node Model::node_0a()
    {
        if (auto node00 = node_00())
            return node00->up.lock();
        return Node{};
    }
    Node Model::node_0b()
    {
        if (auto node00 = node_00())
            return node00->down.lock();
        return Node{};
    }
    Node Model::node_000()
    {
        if (auto node00 = node_00())
            return node00->child.lock();
        return Node{};
    }
    Node Model::node_00a()
    {
        if (auto node000 = node_000())
            return node000->up.lock();
        return Node{};
    }
    Node Model::node_00b()
    {
        if (auto node000 = node_000())
            return node000->down.lock();
        return Node{};
    }

    std::size_t Model::selected_ix(Node &node)
    {
        if (node)
            if (auto child = node->child.lock())
            {
                const auto &childs = node->childs;
                for (auto ix = 0u; ix < childs.size(); ++ix)
                    if (child == childs[ix])
                        return ix;
            }
        return 0;
    }

    bool Model::move(Direction direction, bool me)
    {
        MSS_BEGIN(bool);

        MSS(!!current_node_);

        switch (direction)
        {
            case Direction::Down:
                if (me)
                {
                    if (auto child = current_node_->child.lock())
                        if (auto ptr = child->child.lock())
                        {
                            if (auto down = ptr->down.lock())
                                child->child = down;
                        }
                        else if (auto content = child->content)
                        {
                            if (content->ix+1 < content->items.size())
                                ++content->ix;
                        }
                }
                else
                {
                    if (auto child = current_node_->child.lock())
                    {
                        if (auto down = child->down.lock())
                        {
                            current_node_->child = down;
                        }
                    }
                    else if (auto content = current_node_->content)
                    {
                        if (content->ix+1 < content->items.size())
                            ++content->ix;
                    }
                }
                break;
            case Direction::Up:
                if (me)
                {
                    if (auto child = current_node_->child.lock())
                        if (auto ptr = child->child.lock())
                        {
                            if (auto up = ptr->up.lock())
                                child->child = up;
                        }
                        else if (auto content = child->content)
                        {
                            if (content->ix > 0)
                                --content->ix;
                        }
                }
                else
                {
                    if (auto child = current_node_->child.lock())
                    {
                        if (auto up = child->up.lock())
                            current_node_->child = up;
                    }
                    else if (auto content = current_node_->content)
                    {
                        if (content->ix > 0)
                            --content->ix;
                    }
                }
                break;
            case Direction::Left:
                if (auto ptr = current_node_->parent.lock())
                    current_node_ = ptr;
                break;
            case Direction::Right:
                if (auto ptr = current_node_->child.lock())
                    current_node_ = ptr;
                break;
        }
        MSS_END();
    }

    void Model::recompute_metadata()
    {
        if (root_)
            recompute_metadata_(root_);
    }
    bool Model::sync_metadata()
    {
        MSS_BEGIN(bool);

        MSS(!!root_);
        for (auto &child: root_->childs)
        {
            if (!child)
                continue;
            std::ofstream fo{metadata_fn_(child->path())};
            stream_metadata_(fo, child);
        }

        MSS_END();
    }

    //Privates
    bool Model::save_current_location_()
    {
        MSS_BEGIN(bool);

        std::ofstream fo{current_location_fn_};
        if (auto n = node())
            fo << to_string(n->to_path());

        MSS_END();
    }
    bool Model::load_current_location_()
    {
        MSS_BEGIN(bool);

        current_node_ = root_;

        std::string content;
        MSS(gubg::file::read(content, current_location_fn_));

        for (auto p = to_path(content); !p.empty(); p.pop_back())
            if (focus(p))
                break;

        MSS_END();
    }
    bool Model::add_(Node node, const std::filesystem::path &path, const Config &config)
    {
        MSS_BEGIN(bool);

        assert(!!node);

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
    void Model::recompute_metadata_(Node node)
    {
        assert(!!node);

        node->clear_dependencies();
        for (auto &child: node->childs)
        {
            if (!child)
                continue;

            //Depth-first search
            recompute_metadata_(child);

            node->add_dependencies(child);
        }
    }
    void Model::stream_metadata_(std::ostream &os, Node &node)
    {
        assert(!!node);

        gubg::naft::Document doc{os};
        if (node->metadata.has_local_data())
        {
            auto n = doc.node("Metadata");
            n.attr("path", to_string(node->to_path()));
            node->metadata.stream(n);
        }
        for (auto &child: node->childs)
        {
            if (!child)
                continue;
            //Depth-first search
            stream_metadata_(os, child);
        }
    }
    void Model::compute_navigation_(Node &node)
    {
        assert(!!node);

        Node prev;
        for (auto &child: node->childs)
        {
            if (!child)
                continue;
            if (!prev)
                //TODO: this default navigation setup should be updated with the saved navigation state
                node->child = child;

            child->child.reset();
            child->up = prev;
            child->down.reset();
            if (prev)
                prev->down = child;

            compute_navigation_(child);

            prev = child;
        }
    }

    bool Model::append_metadata_(const std::filesystem::path &fp)
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
    void Model::set_metadata_()
    {
        if (!root_)
            return;
        for (const auto &[path,md]: path__metadata_)
            if (auto n = root_->find(path))
                n->metadata.set_when_unset(md);
    }
    std::filesystem::path Model::metadata_fn_(const std::filesystem::path &base_dir)
    {
        return base_dir / "proast-metadata.naft";
    }

    bool Model::rework_into_directory_(Node node)
    {
        assert(!!node);

        MSS_BEGIN(bool);

        MSS(node->childs.empty());

        const auto orig_fp = node->path();
        MSS(!std::filesystem::is_directory(orig_fp));

        const auto stem = orig_fp.stem();
        const auto ext = orig_fp.extension();
        const auto dir = orig_fp.parent_path()/stem;
        std::filesystem::create_directory(dir);
        auto new_fp = dir/"index";
        new_fp.replace_extension(ext);
        std::filesystem::rename(orig_fp, new_fp);

        auto child = node->append_child();
        child->segment = new_fp.filename();

        node->segment = stem;

        MSS_END();
    }
    bool Model::create_(Node node, const std::string &name, bool create_file)
    {
        assert(!!node);

        MSS_BEGIN(bool);

        if (!std::filesystem::is_directory(node->path()))
            MSS(rework_into_directory_(node));
        MSS(std::filesystem::is_directory(node->path()));

        const auto new_fp = node->path()/name;
        if (create_file)
        {
            std::ofstream touch{new_fp};
        }
        else
        {
            std::filesystem::create_directory(new_fp);
        }
        auto child = node->append_child();
        child->segment = name;

        focus(child->to_path());

        MSS_END();
    }

} } 
