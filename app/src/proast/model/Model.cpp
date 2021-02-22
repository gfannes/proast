#include <proast/model/Model.hpp>
#include <proast/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/mss.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <cassert>
#include <regex>

namespace proast { namespace model { 
    Model::Model()
    {
        root_ = Node_::create(Type::Virtual, "<ROOT>");
        focus_ = {root_};
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

    bool Model::set_home_dir(const std::filesystem::path &home_dir)
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
    std::filesystem::path Model::get_home_dir() const
    {
        return home_dir_;
    }

    bool Model::add_root(const std::filesystem::path &path, const Config &config)
    {
        MSS_BEGIN(bool);

        MSS(std::filesystem::is_directory(path), log::ostream() << "Cannot add " << path << ", this is not a directory" << std::endl);

        root_config_ary_.emplace_back(path, config);

        MSS(!!root_);
        Type t;
        if (false) {}
        else if (std::filesystem::is_regular_file(path)) t = Type::File;
        else if (std::filesystem::is_directory(path)) t = Type::Directory;
        else MSS(false);
        auto child = root_->append_child(t);
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

        MSS(set_home_dir(home_dir_));

        depth_first_search(root_, [](auto &n){n->clear_dependencies();});
        root_ = Node_::create(Type::Virtual, "<ROOT>");

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

    bool Model::focus(const StringPath &path)
    {
        MSS_BEGIN(bool);

        MSS(!!root_);

        MSS(root_->to_node_path(focus_, path));

        for (auto ix = 1u; ix < focus_.size(); ++ix)
            focus_[ix-1]->child = focus_[ix];

        MSS_END();
    }
    bool Model::focus(Node n)
    {
        MSS_BEGIN(bool);
        MSS(!!n);
        MSS(focus(n->to_string_path()));
        MSS_END();
    }

    bool Model::create(const std::string &name)
    {
        MSS_BEGIN(bool);

        auto parent = node_b();
        MSS(!!parent);

        std::string my_name;
        bool create_file = true;
        if (name.empty() || name.back() != '/')
            my_name = name;
        else
        {
            my_name = name.substr(0, name.size()-1);
            create_file = false;
        }

        Node child;
        MSS(create_(child, parent, my_name, create_file));
        parent->child = child;
        focus_.back() = child;

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

            auto copy = p->append_child(n->type);
            copy->segment = new_fp.filename();

            std::filesystem::copy(orig_fp, new_fp, std::filesystem::copy_options::recursive);

            setup_up_down_(p);
            focus(copy);

            if (std::filesystem::is_directory(new_fp))
                MSS(reload());
        }

        MSS_END();
    }
    bool Model::do_export(const std::string &name)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        auto p = n->parent.lock();
        MSS(!!p);
        auto fp = p->path()/name;
        MSS(!std::filesystem::is_regular_file(fp));

        {
            std::ofstream fo{fp};
            auto append_row = [&](auto &node)
            {
                if (auto e = node->metadata.effort)
                    fo << to_string(node->to_string_path(n)) << '\t' << *e << std::endl;
            };
            depth_first_search(n, append_row);
        }

        auto child = p->append_child(Type::File);
        child->segment = fp.filename();

        setup_up_down_(p);

        MSS_END();
    }
    bool Model::search(const std::string &pattern, bool in_content)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        auto p = n->parent.lock();
        MSS(!!p);

        auto child = p->append_child(Type::Virtual);
        child->set_name(std::string("SEARCH:")+pattern);
        setup_up_down_(p);

        std::regex re{pattern};

        auto append_if_matches = [&](auto &node)
        {
            switch (node->type)
            {
                case Type::File:
                case Type::Directory:
                    if (in_content)
                    {
                    }
                    else
                    {
                        if (std::regex_search(node->name(), re))
                        {
                            auto link = child->append_child(Type::Link);
                            link->link = node;
                            link->child = node;
                        }
                    }
                    break;
                default: break;
            }
        };
        depth_first_search(n, append_if_matches);
        for (auto &ch: child->childs)
            child->add_dependencies(ch);
        setup_up_down_(child);
        if (!child->childs.empty())
            child->child = child->childs[0];
        focus(child);

        MSS_END();
    }
    bool Model::plan()
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        const auto plan_name = n->name()+":PLAN";

        auto p = n->parent.lock();
        MSS(!!p);

        Node plan = p->find_child([&](auto child){return child->type == Type::Virtual && child->name() == plan_name;});
        if (!plan)
        {
            plan = p->append_child(Type::Virtual);
            plan->set_name(plan_name);
        }

        auto append_if_has_effort = [&](auto &node)
        {
            if (!node)
                return;
            if (!node->metadata.effort)
                return;
            switch (node->type)
            {
                case Type::File:
                case Type::Directory:
                    {
                        auto link = plan->append_child(Type::Link);
                        link->link = node;
                        link->child = node;
                    }
                    break;
                default: break;
            }
        };
        depth_first_search(n, append_if_has_effort);
        for (auto &ch: plan->childs)
        {
            auto s = log::Scope{"Plan dependencies"};
            plan->add_dependencies(ch);
        }
        setup_up_down_(plan);
        if (!plan->child.lock() && !plan->childs.empty())
            plan->child = plan->childs[0];
        focus(plan);

        setup_up_down_(p);

        MSS_END();
    }
    bool Model::set_node_state(std::optional<State> state, bool done)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        n->metadata.state = state;
        n->metadata.done = done;

        save_metadata();

        MSS_END();
    }
    bool Model::set_order_sequential(bool order_sequential)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        n->metadata.order_sequential = order_sequential;

        save_metadata();

        MSS_END();
    }
    bool Model::append_to_deletes()
    {
        MSS_BEGIN(bool);

        auto n = node_c();
        MSS(!!n);
        auto p = node_b();
        MSS(!!p);

        switch (n->type)
        {
            case Type::File:
            case Type::Directory:
                {
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
                }
                break;
            default:
                MSS(erase_node_(n));
                break;
        }
        focus_.back() = p->child.lock();

        MSS(recompute_metadata());

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
    bool Model::paste()
    {
        MSS_BEGIN(bool);

        auto p = node_b();
        MSS(!!p);

        std::list<Node> problems;
        Node last_ok;
        for (auto del: deletes_)
        {
            if (!del)
                continue;

            //When deleting the node, its segment was reworked into an absolute path
            //Hence del->path() should be OK
            if (!paste_(p, del, del->path()))
                problems.push_back(del);
            else
                last_ok = del;
        }
        deletes_.swap(problems);

        p->child = last_ok;
        focus_.back() = last_ok;

        MSS(deletes_.empty());

        MSS(recompute_metadata());

        MSS_END();
    }


    bool Model::register_bookmark(char ch)
    {
        MSS_BEGIN(bool);

        auto n = node();
        MSS(!!n);

        bookmarks_.set(ch, n->to_string_path());

        MSS_END();
    }
    bool Model::jump_to_bookmark(char ch)
    {
        MSS_BEGIN(bool);

        StringPath path;
        MSS(bookmarks_.get(path, ch));

        MSS(focus(path));

        MSS_END();
    }

    Node Model::node()
    {
        if (auto n = node_c())
            return n;
        if (auto n = node_b())
            return n;
        if (auto n = node_a())
            return n;
        return Node{};
    }
    Node Model::node_a()
    {
        if (focus_.size() < 3)
            return Node{};
        return focus_[focus_.size()-3];
    }
    Node Model::node_b()
    {
        if (focus_.size() < 2)
            return Node{};
        return focus_[focus_.size()-2];
    }
    Node Model::node_b_pre()
    {
        if (auto b = node_b())
            return b->up.lock();
        return Node{};
    }
    Node Model::node_b_post()
    {
        if (auto b = node_b())
            return b->down.lock();
        return Node{};
    }
    Node Model::node_c()
    {
        if (focus_.size() < 1)
            return Node{};
        return focus_[focus_.size()-1];
    }
    Node Model::node_c_pre()
    {
        if (auto c = node_c())
            return c->up.lock();
        return Node{};
    }
    Node Model::node_c_post()
    {
        if (auto c = node_c())
            return c->down.lock();
        return Node{};
    }

    bool Model::move(Movement movement, bool me, bool move_node)
    {
        MSS_BEGIN(bool);
        if (move_node)
        {
            switch (movement)
            {
                case Movement::Up:
                case Movement::Down:
                case Movement::Top:
                case Movement::Bottom:
                    if (focus_.size() >= 2)
                    {
                        auto &b = focus_[focus_.size()-2];
                        auto &c = focus_[focus_.size()-1];
                        if (c)
                        {
                            std::size_t c_ix;
                            MSS(b->get_child_ix(c_ix, c));
                            switch (movement)
                            {
                                case Movement::Up:
                                    if (c_ix > 0)
                                    {
                                        std::swap(b->childs[c_ix], b->childs[c_ix-1]);
                                        c = b->childs[c_ix-1];
                                        b->child = c;
                                    }
                                    break;
                                case Movement::Down:
                                    if (c_ix+1 < b->childs.size())
                                    {
                                        std::swap(b->childs[c_ix], b->childs[c_ix+1]);
                                        c = b->childs[c_ix+1];
                                        b->child = c;
                                    }
                                    break;
                                case Movement::Top:
                                    if (c_ix > 0)
                                    {
                                        auto tmp = b->childs[c_ix];
                                        b->childs.erase(b->childs.begin()+c_ix);
                                        b->childs.insert(b->childs.begin(), tmp);
                                        c = b->childs.front();
                                        b->child = c;
                                    }
                                    break;
                                case Movement::Bottom:
                                    if (c_ix+1 < b->childs.size())
                                    {
                                        auto tmp = b->childs[c_ix];
                                        b->childs.erase(b->childs.begin()+c_ix);
                                        b->childs.push_back(tmp);
                                        c = b->childs.back();
                                        b->child = c;
                                    }
                                    break;
                            }
                        }
                        setup_up_down_(b);
                    }
                    break;

                case Movement::Left:
                    if (focus_.size() >= 3)
                    {
                        auto &a = focus_[focus_.size()-3];
                        auto &b = focus_[focus_.size()-2];
                        auto c = focus_[focus_.size()-1];
                        if (c)
                        {
                            MSS(erase_node_(c));
                            MSS(paste_(a, c, c->path()));
                            b = c;
                            a->child = b;
                        }
                        focus_.pop_back();
                    }
                    break;
                case Movement::Right:
                    if (focus_.size() >= 2)
                    {
                        auto &b = focus_[focus_.size()-2];
                        auto &c = focus_[focus_.size()-1];
                        if (c)
                            if (!std::filesystem::is_directory(c->path()))
                            {
                                MSS(rework_into_directory_(c));
                                if (!c->childs.empty())
                                    focus_.push_back(c->childs.front());
                            }
                    }
                    break;
            }

            //Node order is actual metadata and should be saved
            save_metadata();
        }
        else
        {
            switch (movement)
            {
                case Movement::Up:
                case Movement::Down:
                case Movement::Top:
                case Movement::Bottom:
                    if (focus_.size() >= 2)
                    {
                        auto &b = focus_[focus_.size()-2];
                        auto &c = focus_[focus_.size()-1];
                        if (c)
                        {
                            Node new_c;
                            switch (movement)
                            {
                                case Movement::Up:
                                    new_c = c->up.lock();
                                    break;
                                case Movement::Down:
                                    new_c = c->down.lock();
                                    break;
                                case Movement::Top:
                                    if (!b->childs.empty())
                                        new_c = b->childs.front();
                                    break;
                                case Movement::Bottom:
                                    if (!b->childs.empty())
                                        new_c = b->childs.back();
                                    break;
                            }
                            if (new_c)
                            {
                                b->child = new_c;
                                c = new_c;
                            }
                        }
                    }
                    break;

                case Movement::Left:
                    if (focus_.size() > 1)
                        focus_.pop_back();
                    break;
                case Movement::Right:
                    if (focus_.size() >= 1)
                    {
                        auto &c = focus_[focus_.size()-1];
                        if (c)
                            if (auto ix = c->selected_ix(); ix < c->childs.size())
                                focus_.push_back(c->childs[ix]);
                            else
                                focus_.emplace_back();
                    }
                    break;
            }
        }
        MSS_END();
    }

    bool Model::recompute_metadata()
    {
        MSS_BEGIN(bool);

        MSS(!!root_);
        recompute_metadata_(root_);

        MSS_END();
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
        fo << to_string(to_string_path(focus_));

        MSS_END();
    }
    bool Model::load_current_location_()
    {
        MSS_BEGIN(bool, "");

        focus_ = {root_};

        std::string content;
        MSS(gubg::file::read(content, current_location_fn_));

        for (auto p = to_string_path(content); !p.empty(); p.pop_back())
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
            auto child = node->append_child(is_folder ? Type::Directory : Type::File);
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
        auto child = node->append_child(Type::File);
        child->segment = new_fp.filename();
        setup_up_down_(child);

        node->segment = stem;
        node->child = child;
        node->type = Type::Directory;

        if (auto p = node->parent.lock())
            setup_up_down_(node);

        MSS_END();
    }
    bool Model::create_(Node &child, Node node, const std::string &name, bool create_file)
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
        child = node->append_child(create_file ? Type::File : Type::Directory);
        child->segment = new_fp.filename();

        setup_up_down_(node);

        MSS_END();
    }
    bool Model::erase_node_(Node n)
    {
        MSS_BEGIN(bool);

        auto parent = n->parent.lock();
        MSS(!!parent);

        std::size_t child_ix;
        MSS(parent->get_child_ix(child_ix, n));

        const auto selected_ix = parent->selected_ix();

        auto &childs = parent->childs;

        //Remove ptr from childs
        childs.erase(childs.begin()+child_ix);

        //Set new child selection if we just removed it
        if (selected_ix == child_ix)
        {
            if (child_ix >= childs.size() && child_ix > 0)
                --child_ix;
            if (child_ix >= childs.size())
                parent->child.reset();
            else
                parent->child = childs[child_ix];
        }

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
                n.attr("path", to_string(node->to_string_path(base)));
                node->metadata.stream(n);
            }

            //Order
            {
                auto &childs = node->childs;
                const auto size = childs.size();

                //Collect actual order of names
                actual_order.resize(size);
                actual_order.resize(0);
                for (auto ix = 0u; ix < size; ++ix)
                {
                    auto &child = childs[ix];
                    switch (child->type)
                    {
                        case Type::File:
                        case Type::Directory:
                            actual_order.push_back(child ? child->name() : "");
                            break;
                        default: break;
                    }
                }

                //Default order is sorted on name
                default_order = actual_order;
                std::sort(default_order.begin(), default_order.end());

                if (actual_order != default_order)
                {
                    auto n = doc.node("Order");
                    n.attr("path", to_string(node->to_string_path(base)));
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
        StringPath path;
        std::map<std::string, std::optional<std::size_t>> name__ix;
        for (gubg::naft::Range range{content}; range.pop_tag(tag);)
        {
            if (false) { }
            else if (tag == "Metadata")
            {
                MSS(range.pop_attr(key, value));
                MSS(key == "path");
                path = to_string_path(value);

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
                path = to_string_path(value);

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
