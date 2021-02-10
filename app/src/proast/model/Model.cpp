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
        metadata_filename_ = "proast-metadata.naft";
    }
    Model::~Model()
    {
        if (!bookmarks_.save(bookmarks_fp_))
            log::ostream() << "Warning: could not save bookmarks to " << bookmarks_fp_ << std::endl;
        if (!save_current_location_())
            log::ostream() << "Warning: could not save the current location to " << current_location_fn_ << std::endl;
        if (!save_metadata())
            log::ostream() << "Warning: could not save metadata" << std::endl;
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

        scratchpad_dir_ = home_dir_ / "scratchpad";
        if (!std::filesystem::is_directory(scratchpad_dir_))
            std::filesystem::create_directories(scratchpad_dir_);

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

        load_metadata_(child);
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
            for (auto ix = 0u; node; ++ix)
            {
                if (ix < 3)
                    //A is two levels deeper than the optimal focus point C
                    current_node_ = node;
                auto parent = node->parent.lock();
                if (parent)
                    parent->child = node;
                node = parent;
            }
        }

        MSS_END();
    }
    bool Model::focus(Node n)
    {
        MSS_BEGIN(bool);
        MSS(!!n);
        MSS(focus(n->to_path()));
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

        if (auto p = n->parent.lock())
            setup_up_down_(p);

        MSS_END();
    }
    bool Model::duplicate(const std::string &name)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        if (auto p = n->parent.lock())
        {
            const auto orig_fp = n->path();

            auto new_fp = orig_fp.parent_path()/name;
            //Set same extension as orig_fp, if no extension was specified
            if (std::filesystem::is_regular_file(orig_fp) && new_fp.extension().empty())
                new_fp += orig_fp.extension();
            //Append '_' until we have a non-existing name
            for (auto i = 0; std::filesystem::is_regular_file(new_fp); ++i)
            {
                MSS(i < 10);
                new_fp += '_';
            }

            auto copy = p->append_child();
            copy->segment = new_fp.filename();

            std::filesystem::copy(orig_fp, new_fp, std::filesystem::copy_options::recursive);

            setup_up_down_(p);
            focus(copy);

            if (std::filesystem::is_directory(new_fp))
                MSS(reload());
        }

        MSS_END();
    }
    bool Model::append_to_deletes()
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        const auto orig_fp = n->path();
        MSS(erase_node_(n));
        auto new_fp = scratchpad_dir_/orig_fp.filename();

        //Remove whatever is in the scratchpad in this location
        if (std::filesystem::is_directory(new_fp) || std::filesystem::is_regular_file(new_fp))
            std::filesystem::remove_all(new_fp);

        //Move file/folder to scratchpad
        std::filesystem::rename(orig_fp, new_fp);

        //Indicate full-path in node to make sure we find it back
        n->parent.reset();
        n->segment = new_fp;

        deletes_.push_back(n);

        MSS_END();
    }
    bool Model::clear_deletes()
    {
        MSS_BEGIN(bool);
        for (auto n: deletes_)
            if (n)
                std::filesystem::remove_all(n->path());
        deletes_.clear();
        MSS_END();
    }
    bool Model::paste(bool paste_in)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        if (!paste_in)
            n = n->parent.lock();

        std::list<Node> problems;
        Node last_ok;
        for (auto del: deletes_)
        {
            if (!del)
                continue;

            //When deleting the node, its segment was reworked into an absolute path
            //Hence del->path() should be OK
            if (!paste_(n, del, del->path()))
                problems.push_back(del);
            else
                last_ok = del;
        }
        deletes_.swap(problems);

        MSS(focus(last_ok));

        MSS(deletes_.empty());

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

    bool Model::move(Direction direction, bool me, bool move_node)
    {
        MSS_BEGIN(bool);

        MSS(!!current_node_);

        if (move_node)
        {
            switch (direction)
            {
                case Direction::Down:
                    if (me)
                    {
                        if (auto child = current_node_->child.lock())
                            if (auto ix = selected_ix(child); ix+1 < child->childs.size())
                            {
                                std::swap(child->childs[ix], child->childs[ix+1]);
                                setup_up_down_(child);
                            }
                    }
                    else
                    {
                        if (auto ix = selected_ix(current_node_); ix+1 < current_node_->childs.size())
                        {
                            std::swap(current_node_->childs[ix], current_node_->childs[ix+1]);
                            setup_up_down_(current_node_);
                        }
                    }
                    break;
                case Direction::Up:
                    if (me)
                    {
                        if (auto child = current_node_->child.lock())
                            if (auto ix = selected_ix(child); ix > 0)
                            {
                                std::swap(child->childs[ix], child->childs[ix-1]);
                                setup_up_down_(child);
                            }
                    }
                    else
                    {
                        if (auto ix = selected_ix(current_node_); ix > 0)
                        {
                            std::swap(current_node_->childs[ix], current_node_->childs[ix-1]);
                            setup_up_down_(current_node_);
                        }
                    }
                    break;
                case Direction::Left:
                    if (me)
                    {
                        if (auto child = current_node_->child.lock())
                            if (auto ptr = child->child.lock())
                            {
                                MSS(erase_node_(ptr));
                                MSS(paste_(current_node_, ptr, ptr->path()));
                                MSS(focus(ptr));
                            }
                    }
                    break;
                case Direction::Right:
                    if (me)
                    {
                        if (auto child = current_node_->child.lock())
                            if (auto ptr = child->child.lock())
                                if (!std::filesystem::is_directory(ptr->path()))
                                {
                                    MSS(rework_into_directory_(ptr));
                                    current_node_ = child;
                                }
                    }
                    break;
            }

            //Node order is actual metadata and should be saved
            save_metadata();
        }
        else
        {
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
        }
        MSS_END();
    }

    void Model::recompute_metadata()
    {
        if (root_)
            recompute_metadata_(root_);
    }
    bool Model::save_metadata()
    {
        MSS_BEGIN(bool);

        MSS(!!root_);
        unsigned int nr_problems = 0;
        for (auto &child: root_->childs)
        {
            if (!child)
                continue;
            if (!save_metadata_(child))
                ++nr_problems;
        }
        MSS(nr_problems == 0);

        MSS_END();
    }
    bool Model::load_metadata()
    {
        MSS_BEGIN(bool);

        MSS(!!root_);
        unsigned int nr_problems = 0;
        for (auto &child: root_->childs)
        {
            if (!child)
                continue;
            if (!load_metadata_(child))
                ++nr_problems;
        }
        MSS(nr_problems == 0);

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

        //Create new child for the "index" file
        auto child = node->append_child();
        child->segment = new_fp.filename();
        setup_up_down_(child);

        node->segment = stem;
        node->child = child;

        if (auto p = node->parent.lock())
            setup_up_down_(node);

        MSS_END();
    }
    bool Model::create_(Node node, const std::string &name, bool create_file)
    {
        assert(!!node);

        MSS_BEGIN(bool);

        if (!std::filesystem::is_directory(node->path()))
            MSS(rework_into_directory_(node));
        MSS(std::filesystem::is_directory(node->path()));

        auto new_fp = node->path()/name;
        if (create_file)
        {
            if (new_fp.extension().empty())
                new_fp += ".md";
            std::ofstream touch{new_fp};
        }
        else
        {
            std::filesystem::create_directory(new_fp);
        }
        auto child = node->append_child();
        child->segment = new_fp.filename();

        setup_up_down_(node);
        focus(child);

        MSS_END();
    }
    bool Model::erase_node_(Node n)
    {
        MSS_BEGIN(bool);

        auto parent = n->parent.lock();
        MSS(!!parent);
        auto ix = selected_ix(parent);
        auto &childs = parent->childs;

        //Remove ptr from childs
        childs.erase(childs.begin()+ix);

        //Set new child selection
        if (ix >= childs.size() && ix > 0)
            --ix;
        if (ix >= childs.size())
            parent->child.reset();
        else
            parent->child = childs[ix];

        setup_up_down_(parent);

        MSS_END();
    }
    bool Model::paste_(Node dst, Node src, const std::filesystem::path &orig_fp)
    {
        assert(!!dst);
        assert(!!src);

        MSS_BEGIN(bool);

        if (!std::filesystem::is_directory(dst->path()))
            MSS(rework_into_directory_(dst));

        auto orig_parent = src->parent.lock();
        src->parent = dst;
        std::string suffix;
        for (bool ok = false; !ok; suffix += '_')
        {
            MSS(suffix.size() < 10, src->parent = orig_parent);

            //Create segment name as original filename with a few '_' for the retries when the
            //name is already occupied
            src->segment = orig_fp.filename();
            src->segment += suffix;

            const auto new_fp = src->path();
            if (!std::filesystem::is_directory(new_fp) && !std::filesystem::is_regular_file(new_fp))
            {
                std::filesystem::rename(orig_fp, new_fp);
                ok = true;
            }
        }

        dst->childs.push_back(src);
        setup_up_down_(dst);

        MSS_END();
    }
    void Model::setup_up_down_(Node &n)
    {
        assert(!!n);

        Node prev;
        for (auto &child: n->childs)
        {
            if (!child)
                continue;
            child->up = prev;
            child->down.reset();
            if (prev)
                prev->down = child;
            prev = child;
        }
    }

    bool Model::save_metadata_(Node base)
    {
        assert(!!base);

        MSS_BEGIN(bool);

        const auto fp = base->path()/metadata_filename_;
        auto s = log::Scope{"save_metadata_()", [&](auto &hdr){hdr.attr("name", base->name()).attr("filepath", fp.string());}};

        std::ofstream fo{fp};
        gubg::naft::Document doc{fo};

        std::vector<std::string> default_order, actual_order;
        auto ftor = [&](const Node &node)
        {
            //Metadata
            if (node->metadata.has_local_data())
            {
                log::ostream() << "Saving MD for " << node->path() << std::endl;
                auto n = doc.node("Metadata");
                n.attr("path", to_string(node->to_path(base)));
                node->metadata.stream(n);
            }

            //Order
            {
                auto &childs = node->childs;
                const auto size = childs.size();

                //Collect actual order of names
                actual_order.resize(size);
                for (auto ix = 0u; ix < size; ++ix)
                {
                    auto &child = childs[ix];
                    actual_order[ix] = child ? child->name() : "";
                }

                //Default order is sorted on name
                default_order = actual_order;
                std::sort(default_order.begin(), default_order.end());

                if (actual_order != default_order)
                {
                    auto n = doc.node("Order");
                    n.attr("path", to_string(node->to_path(base)));
                    for (auto &name: actual_order)
                        n.attr("name", name);
                }
            }
        };
        depth_first_search(base, ftor);

        MSS_END();
    }
    bool Model::load_metadata_(Node base)
    {
        assert(!!base);

        MSS_BEGIN(bool);

        const auto fp = base->path()/metadata_filename_;
        auto s = log::Scope{"load_metadata_()", [&](auto &hdr){hdr.attr("name", base->name()).attr("filename", fp);}};

        std::string content;
        MSS(gubg::file::read(content, fp));

        std::string tag, key, value;
        Path path;
        std::map<std::string, std::optional<std::size_t>> name__ix;
        for (gubg::naft::Range range{content}; range.pop_tag(tag);)
        {
            if (false) { }
            else if (tag == "Metadata")
            {
                MSS(range.pop_attr(key, value));
                MSS(key == "path");
                path = to_path(value);

                gubg::naft::Range subrange;
                MSS(range.pop_block(subrange));
                Metadata md;
                MSS(md.parse(subrange));

                if (auto n = base->find(path); !n)
                    log::ostream() << "Error: could not find node " << to_string(path) << std::endl;
                else
                    n->metadata.set_when_unset(md);
            }
            else if (tag == "Order")
            {
                MSS(range.pop_attr(key, value));
                MSS(key == "path");
                path = to_path(value);

                name__ix.clear();
                for (auto ix = 0u; range.pop_attr(key, value); ++ix)
                {
                    MSS(key == "name");
                    name__ix[value] = ix;
                }

                if (auto n = base->find(path); !n)
                    log::ostream() << "Error: could not find node " << to_string(path) << std::endl;
                else
                {
                    auto cmp = [&](const Node &a, const Node &b)
                    {
                        auto aix = name__ix[a ? a->name() : ""];
                        auto bix = name__ix[b ? b->name() : ""];
                        if (aix && bix)
                            return *aix < *bix;
                        if (aix)
                            return true;
                        return false;
                    };
                    std::sort(n->childs.begin(), n->childs.end(), cmp);

                    setup_up_down_(n);
                }
            }
            else
            {
                MSS(false, log::ostream() << "Error: unknown item [" << tag << "] found in " << fp << std::endl);
            }
        }

        MSS_END();
    }

} } 
