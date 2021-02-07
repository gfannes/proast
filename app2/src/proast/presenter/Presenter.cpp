#include <proast/presenter/Presenter.hpp>
#include <proast/util.hpp>
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

        {
            oss_.str("");
            if (auto n = model_.node())
                oss_ << n->value.path.string();
            else
                oss_ << "<no valid node>";
            view_.header = oss_.str();
        }

        {
            auto set_view_dto = [&](auto &lst, auto node)
            {
                if (!node)
                {
                    lst.reset();
                    return;
                }

                if (std::filesystem::is_regular_file(node->value.path))
                {
                    if (!node->value.content)
                        node->value.content = content_mgr_.load(node->value.path);
                    lst = node->value.content;
                }
                else
                {
                    lst = dto::List::create();
                    for (auto &n: node->childs.nodes)
                    {
                        oss_.str("");
                        if (show_metadata_field_)
                        {
                            switch (*show_metadata_field_)
                            {
                                case MetadataField::Effort:
                                    oss_ << std::fixed << std::setprecision(1) << std::setw(5);
                                    if (auto effort = n.value.metadata.effort(); effort > 0)
                                        oss_ << effort;
                                    else
                                        oss_ << ' ';
                                    oss_ << ' ';
                                    break;
                                case MetadataField::Dependency:
                                    oss_ << std::fixed << std::setprecision(1) << std::setw(4);
                                    oss_ << n.value.metadata.dependencies.size() << " ";
                                    break;
                            }
                        }
                        else
                            oss_ << ' ';
                        oss_ << n.value.name;
                        lst->items.emplace_back(oss_.str());
                    }
                    lst->ix = model::Tree::selected_ix(*node);
                }

                lst->name = model::to_string(to_path(node));
            };
            set_view_dto(view_.n0,   model_.node_0());
            set_view_dto(view_.n0a,  model_.node_0a());
            set_view_dto(view_.n00,  model_.node_00());
            set_view_dto(view_.n0b,  model_.node_0b());
            set_view_dto(view_.n00a, model_.node_00a());
            set_view_dto(view_.n000, model_.node_000());
            set_view_dto(view_.n00b, model_.node_00b());

            {
                auto lst = dto::List::create();
                if (auto node = model_.node())
                {
                    lst->name = model::to_string(to_path(node));

                    const unsigned int align = 15;
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
                    add_field(node->value.metadata.dependencies, "dependencies", as_size);
                    add_field(node->value.metadata.effort(), "effort", as_number);
                    add_tags(node->value.metadata.tags(), "tags");

                    add_field_opt(node->value.metadata.my_effort, "my effort", as_number);
                    add_field_opt(node->value.metadata.my_impact, "my impact", as_number);
                    add_field_opt(node->value.metadata.my_completion_pct, "my completion", as_pct);
                    add_field_opt(node->value.metadata.my_volume_db, "my volume", as_volume);
                    add_field_opt(node->value.metadata.my_live, "my live", as_date);
                    add_field_opt(node->value.metadata.my_dead, "my dead", as_date);
                    add_tags(node->value.metadata.my_tags, "my tags");
                }
                view_.metadata = lst;
            }

            {
                auto lst = dto::List::create();
                if (Commander::state)
                    switch (*Commander::state)
                    {
                        case State::BookmarkRegister:
                        case State::BookmarkJump:
                            lst->name = "Bookmarks";
                            model_.each_bookmark([&](auto ch, const auto &path){
                                    oss_.str("");
                                    oss_ << ch << " => " << model::to_string(path);
                                    lst->items.emplace_back(oss_.str());
                                    });
                            break;
                        case State::SetMetadataField:
                        case State::ShowMetadataField:
                            lst->name = "Metadata fields";
                            for (auto ch: {'e', 'v', 'i', 'c', 'l', 'd', 't', 'D'})
                            {
                                oss_.str("");
                                oss_ << ch << ": ";
                                if (auto mf = to_metadata_field(ch))
                                    oss_ << to_string(*mf);
                                lst->items.emplace_back(oss_.str());
                            }
                            break;
                        case State::Create:
                            {
                                lst->name = "Create";
                                auto add_help = [&](auto ch, auto descr)
                                {
                                    oss_.str("");
                                    oss_ << ch << ": " << descr;
                                    lst->items.emplace_back(oss_.str());
                                };
                                add_help('f', "Create file in parent");
                                add_help('F', "Create file in self");
                                add_help('d', "Create directory in parent");
                                add_help('D', "Create directory in self");
                            }
                            break;
                        default: break;
                    }
                view_.details = lst;
            }
        }

        {
            oss_.str("");
            if (Commander::state)
                switch (*Commander::state)
                {
                    case State::BookmarkRegister:  oss_ << "Registering bookmark"; break;
                    case State::BookmarkJump:      oss_ << "Jump to bookmark"; break;
                    case State::SetMetadataField:
                                                   if (!Commander::metadata_field)
                                                       oss_ << "Choose metadata field";
                                                   else
                                                       oss_ << "Metadata for " << to_string(*Commander::metadata_field) << ": " << Commander::content;
                                                   break;
                    case State::ShowMetadataField: oss_ << "Show metadata field"; break;
                    case State::Create:            if (!Commander::create_what)
                                                       oss_ << "Create new file or directory";
                                                   else
                                                       oss_ << "Name: " << Commander::content;
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
    void Presenter::commander_move(Direction direction, bool me)
    {
        model_.move(direction, me);
    }
    void Presenter::commander_open(Open open)
    {
        if (auto me = model_.node_000())
        {
            const auto path = me->value.path;
            scheduled_operation_ = [path,open]()
            {
                const auto orig_dir = std::filesystem::current_path();
                std::filesystem::current_path(path.parent_path());

                std::ostringstream oss;
                switch (open)
                {
                    case Open::View:
                        oss << "nvim " << path.filename();
                        break;
                    case Open::Edit:
                        {
                            std::string my_editor = "nvim";
                            if (auto editor = std::getenv("EDITOR"))
                                my_editor = editor;
                            oss << my_editor << " " << path.filename();
                        }
                        break;
                    case Open::Shell:
                        oss << "bash";
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
                case MetadataField::Effort:        as_number(node->value.metadata.my_effort, 1.0); break;
                case MetadataField::Volume:        as_number(node->value.metadata.my_volume_db, -20.0); break;
                case MetadataField::Impact:        as_number(node->value.metadata.my_impact, 1); break;
                case MetadataField::CompletionPct: as_number(node->value.metadata.my_completion_pct, 0.0); break;
                case MetadataField::Live:          as_date(node->value.metadata.my_live); break;
                case MetadataField::Dead:          as_date(node->value.metadata.my_dead); break;
                case MetadataField::Tag:
                                                   if (content.empty())
                                                       was_set = true;
                                                   else if (content == "~")
                                                       node->value.metadata.my_tags.clear();
                                                   else if (content[0] == '~')
                                                       node->value.metadata.my_tags.erase(content.substr(1));
                                                   else
                                                       node->value.metadata.my_tags.insert(content);
                                                   break;
            }
            if (was_set)
            {
                model_.sync_metadata();
                model_.recompute_metadata();
            }
        }
    }
    void Presenter::commander_show_metadata(std::optional<MetadataField> mf_opt)
    {
        show_metadata_field_ = mf_opt;
    }
    void Presenter::commander_create(const std::string &name, bool create_file, bool in_parent)
    {
        if (create_file)
            model_.create_file(name, in_parent);
        else
            model_.create_folder(name, in_parent);
    }
    void Presenter::commander_delete()
    {
        model_.delete_current();
    }
    void Presenter::commander_reload()
    {
        model_.reload();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        auto s = log::Scope{"Presenter::received()", [=](auto &hdr){
            hdr.attr("uint", (unsigned int)wchar);
            if (32 <= wchar && wchar < 128) hdr.attr("char", (char)wchar);
        }};

        Commander::process(wchar);
        refresh_view_();
    }
} } 
