#include <proast/presenter/Presenter.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <fstream>
#include <algorithm>

namespace proast { namespace presenter { 
    Presenter::Presenter(model::Model &model, view::View &view): model_(model), view_(view)
    {
        view_.events = this;
    }

    bool Presenter::run()
    {
        MSS_BEGIN(bool);

        for (bool do_run = true; do_run;)
        {
            MSS(refresh_view_());

            view_.run();

            do_run = false;
            if (scheduled_operation_)
            {
                MSS(scheduled_operation_());
                MSS(model_.reload());
                scheduled_operation_ = nullptr;
                do_run = true;
            }
        }

        MSS_END();
    }

    bool Presenter::refresh_view_()
    {
        MSS_BEGIN(bool);

        auto s = log::Scope{"Presenter::refresh_view_()"};

        //Header
        {
            oss_.str("");
            if (auto n = model_.node())
                oss_ << model::to_string(n->to_string_path());
            else
                oss_ << "<no valid node>";
            view_.header = oss_.str();
        }

        //Panes
        {
            auto set_view_dto = [&](auto &lst, auto node)
            {
                if (!node)
                {
                    lst.reset();
                    return;
                }

                auto type__attention = [](auto t){
                    switch (t)
                    {
                        case model::Type::File: return 3;
                        case model::Type::Directory: return 5;
                        case model::Type::Virtual: return 4;
                        case model::Type::Link: return 6;
                        default: break;
                    }
                    return 0;
                };

                if (auto rnode = node->resolve())
                    switch (rnode->type)
                    {
                        case model::Type::File:
                            if (!rnode->content)
                                rnode->content = content_mgr_.load(rnode->path());
                            lst = rnode->content;
                            break;
                        case model::Type::Directory:
                        case model::Type::Virtual:
                            lst = dto::List::create();
                            for (auto &child: rnode->childs)
                            {
                                if (!child)
                                    continue;
                                oss_.str("");
                                unsigned int width = 0;
                                if (show_metadata_field_)
                                {
                                    switch (*show_metadata_field_)
                                    {
                                        case MetadataField::Effort:
                                            width = 5;
                                            oss_ << std::fixed << std::setprecision(1) << std::setw(width);
                                            if (auto effort = child->total_effort(); effort > 0)
                                                oss_ << effort;
                                            else
                                                oss_ << ' ';
                                            oss_ << ' ';
                                            break;
                                        case MetadataField::Dependency:
                                            width = 4;
                                            oss_ << std::fixed << std::setprecision(1) << std::setw(width);
                                            oss_ << child->dependency_count() << " ";
                                            break;
                                    }
                                }
                                else
                                    oss_ << ' ';
                                oss_ << child->name();
                                lst->items.emplace_back(oss_.str());
                                lst->items.back().ix__attention[width] = type__attention(child->type);
                            }
                            lst->ix = rnode->selected_ix();
                            break;
                    }
                else
                {
                    lst = dto::List::create();
                    lst->items.emplace_back("<could not resolve node>");
                }

                lst->name = model::to_string(node->to_string_path());
                lst->name.ix__bold[0] = true;
                lst->name.ix__attention[0] = type__attention(node->type);
            };
            set_view_dto(view_.n0,   model_.node_a());
            set_view_dto(view_.n0a,  model_.node_b_pre());
            set_view_dto(view_.n00,  model_.node_b());
            set_view_dto(view_.n0b,  model_.node_b_post());
            set_view_dto(view_.n00a, model_.node_c_pre());
            set_view_dto(view_.n000, model_.node_c());
            set_view_dto(view_.n00b, model_.node_c_post());

            //Metadata
            {
                auto lst = dto::List::create();
                lst->name.ix__attention[0] = 2;
                lst->name.ix__bold[0] = true;
                if (auto node = model_.node())
                {
                    lst->name = model::to_string(node->to_string_path());

                    const unsigned int align = 10;
                    auto as_number = [](auto &os, const auto &effort){
                        os << std::fixed << std::setprecision(1) << effort;
                    };
                    auto as_size = [](auto &os, const auto &container){
                        os << container.size();
                    };
                    auto as_volume = [](auto &os, const auto &volume){
                        os << std::fixed << std::setprecision(0) << volume << "dB";
                    };
                    auto as_pct = [](auto &os, const auto &pct){
                        os << std::fixed << std::setprecision(0) << pct << "%";
                    };
                    auto as_date = [](auto &os, const auto &date){
                        os << date;
                    };
                    auto as_state = [](auto &os, const auto &state){
                        os << state;
                    };
                    auto as_bool = [](auto &os, bool done){
                        os << (done ? "yes" : "no");
                    };
                    auto as_order = [](auto &os, bool order_sequential){
                        os << (order_sequential ? "Sequential" : "Random");
                    };
                    auto add_field = [&](const auto &value, const auto descr, auto &&streamer){
                        oss_.str("");
                        oss_ << std::setw(align) << descr << ": ";
                        streamer(oss_, value);
                        lst->items.push_back(oss_.str());
                    };
                    auto add_field_opt = [&](const auto &value, const auto descr, auto &&streamer){
                        if (!value)
                            return;
                        add_field(value.value(), descr, streamer);
                    };
                    auto add_tags = [&](const auto &tags, const auto descr)
                    {
                        if (tags.size())
                        {
                            oss_.str("");
                            oss_ << std::setw(align) << descr << ": ";
                            for (const auto &tag: tags)
                                oss_ << tag << " ";
                            lst->items.push_back(oss_.str());
                        }
                    };
                    add_field_opt(node->metadata.effort, "effort", as_number);
                    add_field_opt(node->metadata.impact, "impact", as_number);
                    add_field_opt(node->metadata.completion_pct, "completion", as_pct);
                    add_field_opt(node->metadata.volume_db, "volume", as_volume);
                    add_field_opt(node->metadata.live, "live", as_date);
                    add_field_opt(node->metadata.dead, "dead", as_date);
                    add_field_opt(node->metadata.state, "state", as_state);
                    add_field(node->metadata.done, "done", as_bool);
                    add_field(node->metadata.order_sequential, "order", as_order);
                    add_field_opt(node->priority(), "prio", as_number);
                    add_tags(node->metadata.tags, "tags");
                    add_tags(node->all_tags(), "All tags");
                }
                view_.metadata = lst;
            }

            //Details
            {
                auto lst = dto::List::create();
                lst->name.ix__attention[0] = 2;
                lst->name.ix__bold[0] = true;
                auto add_help = [&](auto ch, auto descr)
                {
                    oss_.str("");
                    if (ch != '\0')
                        oss_ << ch << ": ";
                    oss_ << descr;
                    lst->items.emplace_back(oss_.str());
                    lst->items.back().ix__attention[0] = 7;
                };
                if (Commander::state)
                    switch (*Commander::state)
                    {
                        case State::BookmarkRegister:
                        case State::BookmarkJump:
                            lst->name = "Bookmarks";
                            model_.each_bookmark([&](auto ch, const auto &path){add_help(ch, model::to_string(path));});
                            break;
                        case State::SetData:
                            lst->name = "Data";
                            add_help('m', "Metadata");
                            add_help('s', "State");
                            add_help('o', "Order");
                            break;
                        case State::SetMetadata:
                        case State::ShowMetadata:
                            lst->name = "Metadata fields";
                            for (auto ch: {'e', 'v', 'i', 'c', 'l', 'd', 't', 'D'})
                                if (auto mf = to_metadata(ch))
                                    add_help(ch, to_string(*mf));
                            add_help('~', "Erase metadata field");
                            break;
                        case State::SetState:
                            {
                                std::optional<model::State> state;
                                bool done;
                                lst->name = "State";
                                for (auto ch: {'c','C', 'd','D', 's','S', 'i','I', 'v','V'})
                                    if (to_node_state(state, done, ch))
                                        if (state)
                                            add_help(ch, std::string(done ? "DONE " : "WIP  ")+to_string(*state));
                                add_help('~', "Erase state field");
                            }
                            break;
                        case State::SetOrder:
                            lst->name = "Order";
                            for (auto ch: {'r', 's'})
                                if (auto o = to_order_sequential(ch))
                                    if (o)
                                        add_help(ch, std::string(*o ? "Sequential" : "Random"));
                            break;
                        case State::Create:
                            lst->name = "Create";
                            add_help('f', "Create file");
                            add_help('d', "Create directory");
                            add_help('i', "Create file/directory in node");
                            add_help('n', "Create file/directory next to node");
                            break;
                        case State::Delete:
                            lst->name = "Delete";
                            add_help('d', "Delete single file/directory");
                            add_help('a', "Append file/directory to deletes");
                            add_help('c', "Clear all deletes");
                            break;
                        case State::Paste:
                            lst->name = "Paste";
                            add_help('i', "Paste file/directory in node");
                            add_help('n', "Paste file/directory next to node");
                            break;
                        case State::Rename:
                            lst->name = "Rename";
                            add_help('\0', "Specify the new name");
                            break;
                        case State::Run:
                            lst->name = "Run";
                            add_help('\0', "Specify the executable to use");
                            break;
                        case State::Duplicate:
                            lst->name = "Duplicate";
                            add_help('\0', "Specify the name for the copy");
                            break;
                        case State::Export:
                            lst->name = "Export";
                            add_help('\0', "Specify the name for the export");
                            break;
                        case State::Search:
                            lst->name = "Search";
                            add_help('\0', "^/: Search pattern for file and folder names");
                            add_help('/', "Search pattern for file content");
                            break;
                        default: break;
                    }
                else
                {
                    unsigned int nr_deletes = 0;
                    model_.each_delete([&](auto &n){++nr_deletes;});
                    if (nr_deletes > 0)
                    {
                        oss_.str("");
                        oss_ << "Deletes: " << nr_deletes;
                        lst->name = oss_.str();
                        model_.each_delete([&](auto &n){lst->items.emplace_back(n->name());});
                    }
                }
                view_.details = lst;
            }
        }

        //Footer
        {
            oss_.str("");
            if (Commander::state)
                switch (*Commander::state)
                {
                    case State::BookmarkRegister:  oss_ << "Registering bookmark"; break;
                    case State::BookmarkJump:      oss_ << "Jump to bookmark"; break;
                    case State::SetData:           oss_ << "Choose data type to set"; break;
                    case State::SetMetadata:
                                                   if (!Commander::metadata_field)
                                                       oss_ << "Choose metadata field";
                                                   else
                                                       oss_ << "Metadata for " << to_string(*Commander::metadata_field) << ": " << Commander::content;
                                                   break;
                    case State::ShowMetadata:      oss_ << "Show metadata field"; break;
                    case State::Create:            if (!Commander::create_file_dir)
                                                       oss_ << "Create new file or directory?";
                                                   else if (!Commander::create_in_next)
                                                       oss_ << "Create in or next to current node?";
                                                   else
                                                       oss_ << "Name: " << Commander::content;
                                                   break;
                    case State::Rename:            oss_ << "Name: " << Commander::content; break;
                    case State::Run:               oss_ << "Executable: " << Commander::content; break;
                    case State::Duplicate:         oss_ << "Duplicate: " << Commander::content; break;
                    case State::Export:            oss_ << "Export: " << Commander::content; break;
                    case State::Search:            if (Commander::content.empty() || Commander::content[0] != '/')
                                                       oss_ << "Search in names: ";
                                                   else
                                                       oss_ << "Search in content: ";
                                                   oss_ << Commander::content;
                                                   break;
                    default: break;
                }
            view_.footer = oss_.str();
        }

        MSS_END();
    }

    //Commander API
    void Presenter::commander_quit()
    {
        view_.quit();
    }
    void Presenter::commander_move(Movement movement, bool me, bool move_node)
    {
        model_.move(movement, me, move_node);
    }
    void Presenter::commander_open(Open open, std::string cmd)
    {
        if (auto me = model_.node_c())
        {
            const auto path = me->path();
            scheduled_operation_ = [=]() mutable
            {
                const auto orig_dir = std::filesystem::current_path();
                std::filesystem::current_path(path.parent_path());

                std::ostringstream oss;
                switch (open)
                {
                    case Open::View:
                        if (cmd.empty())
                            cmd = "nvim";
                        oss << cmd << " " << path.filename();
                        break;
                    case Open::Edit:
                        {
                            if (cmd.empty())
                            {
                                cmd = "nvim";
                                if (auto editor = std::getenv("EDITOR"))
                                    cmd = editor;
                            }
                            oss << cmd << " " << path.filename();
                        }
                        break;
                    case Open::Shell:
                        if (cmd.empty())
                            cmd = "bash";
                        oss << cmd;
                        break;
                    case Open::Run:
                        if (cmd.empty())
                            oss << "./" << path.filename();
                        else
                            oss << cmd << " " << path.filename();
                        break;
                }

                log::raw([&](auto &os){os << "Running \"" << oss.str() << "\"\n";});
                const auto rc = std::system(oss.str().c_str());
                log::raw([&](auto &os){os << "  => " << rc << "\n";});

                std::filesystem::current_path(orig_dir);

                return true;
            };
            view_.quit();
        }
    }
    void Presenter::commander_bookmark(char ch, bool do_register)
    {
        if (do_register)
            model_.register_bookmark(ch);
        else
            model_.jump_to_bookmark(ch);
    }
    void Presenter::commander_set_metadata(MetadataField field, const std::string &content)
    {
        if (auto node = model_.node())
        {
            bool was_set = true;
            auto as_number = [&](auto &dst, auto default_value)
            {
                try
                {
                    if (content.empty())
                        dst = default_value;
                    else if (content[0] == '~')
                        dst.reset();
                    else
                        dst = std::stod(content);
                }
                catch (std::invalid_argument) { was_set = false; }
            };
            auto as_date = [&](auto &dst)
            {
                if (content.empty())
                    dst = "now";
                else if (content[0] == '~')
                    dst.reset();
                else
                    dst = content;
            };
            switch (field)
            {
                case MetadataField::Effort:        as_number(node->metadata.effort, 1.0); break;
                case MetadataField::Volume:        as_number(node->metadata.volume_db, -20.0); break;
                case MetadataField::Impact:        as_number(node->metadata.impact, 1); break;
                case MetadataField::CompletionPct: as_number(node->metadata.completion_pct, 0.0); break;
                case MetadataField::Live:          as_date(node->metadata.live); break;
                case MetadataField::Dead:          as_date(node->metadata.dead); break;
                case MetadataField::Tag:
                                                   if (content.empty())
                                                       was_set = true;
                                                   else if (content == "~")
                                                       node->metadata.tags.clear();
                                                   else if (content[0] == '~')
                                                       node->metadata.tags.erase(content.substr(1));
                                                   else
                                                       node->metadata.tags.insert(content);
                                                   break;
            }
            if (was_set)
            {
                model_.save_metadata();
                model_.recompute_metadata();
            }
        }
    }
    void Presenter::commander_show_metadata(std::optional<MetadataField> mf_opt)
    {
        show_metadata_field_ = mf_opt;
    }
    void Presenter::commander_create(const std::string &name, bool create_file, bool create_in)
    {
        model_.create(name, create_file, create_in);
    }
    void Presenter::commander_rename(const std::string &name)
    {
        model_.rename(name);
    }
    void Presenter::commander_duplicate(const std::string &name)
    {
        model_.duplicate(name);
    }
    void Presenter::commander_delete(Delete del)
    {
        switch (del)
        {
            case Delete::One:    model_.clear_deletes(); model_.append_to_deletes(); break;
            case Delete::Append:                         model_.append_to_deletes(); break;
            case Delete::Clear:  model_.clear_deletes();                             break;
        }
    }
    void Presenter::commander_paste(bool paste_in)
    {
        model_.paste(paste_in);
    }
    void Presenter::commander_export(const std::string &name)
    {
        model_.do_export(name);
    }
    void Presenter::commander_search(const std::string &pattern, bool in_content)
    {
        model_.search(pattern, in_content);
    }
    void Presenter::commander_plan()
    {
        model_.plan();
    }
    void Presenter::commander_set_node_state(std::optional<model::State> state, bool done)
    {
        model_.set_node_state(state, done);
    }
    void Presenter::commander_set_order(bool order_sequential)
    {
        model_.set_order_sequential(order_sequential);
    }
    void Presenter::commander_reload()
    {
        model_.reload();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar, bool alt)
    {
        auto s = log::Scope{"Presenter::received()", [=](auto &hdr){
            hdr.attr("uint", (unsigned int)wchar);
            if (32 <= wchar && wchar < 128) hdr.attr("char", (char)wchar);
        }};

        Commander::process(wchar, alt);
        refresh_view_();
    }
} } 
