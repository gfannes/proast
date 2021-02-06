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

        {
            oss_.str(L"");
            if (auto n = model_.node())
                oss_ << n->value.path.wstring();
            else
                oss_ << L"<no valid node>";
            view_.header = oss_.str();
        }

        {
            auto set_view_dto = [&](auto &lst, auto node, std::size_t path_size)
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
                        oss_.str(L"");
                        if (show_metadata_field_)
                        {
                            switch (*show_metadata_field_)
                            {
                                case MetadataField::Effort:
                                    oss_ << std::fixed << std::setprecision(1) << std::setw(5);
                                    if (auto effort = n.value.metadata.effort(); effort > 0)
                                        oss_ << effort;
                                    else
                                        oss_ << L' ';
                                    oss_ << L' ';
                                    break;
                                case MetadataField::Dependency:
                                    oss_ << std::fixed << std::setprecision(1) << std::setw(4);
                                    oss_ << n.value.metadata.dependencies.size() << L" ";
                                    break;
                            }
                        }
                        else
                            oss_ << L' ';
                        oss_ << n.value.name;
                        lst->items.emplace_back(oss_.str());
                    }
                    lst->ix = model::Tree::selected_ix(*node);
                }

                auto path = to_path(node);
                path_size = std::min(path_size, path.size());
                path.erase(path.begin(), path.end()-path_size);

                lst->name = model::to_wstring(path);
            };
            set_view_dto(view_.n0,   model_.node_0(),   1);
            set_view_dto(view_.n0a,  model_.node_0a(),  2);
            set_view_dto(view_.n00,  model_.node_00(),  2);
            set_view_dto(view_.n0b,  model_.node_0b(),  2);
            set_view_dto(view_.n00a, model_.node_00a(), 3);
            set_view_dto(view_.n000, model_.node_000(), 3);
            set_view_dto(view_.n00b, model_.node_00b(), 3);

            {
                auto lst = dto::List::create();
                if (auto node = model_.node())
                {
                    lst->name = L"Metadata for ";
                    auto path = to_path(node);
                    const auto path_size = std::min<unsigned int>(3, path.size());
                    path.erase(path.begin(), path.end()-path_size);
                    lst->name += model::to_wstring(path);

                    const unsigned int align = 15;
                    auto as_number = [](auto &os, const auto &effort){
                        os << std::fixed << std::setprecision(1) << effort;
                    };
                    auto as_size = [](auto &os, const auto &container){
                        os << container.size();
                    };
                    auto as_volume = [](auto &os, const auto &volume){
                        os << std::fixed << std::setprecision(0) << volume << L"dB";
                    };
                    auto as_pct = [](auto &os, const auto &pct){
                        os << std::fixed << std::setprecision(0) << pct << L"%";
                    };
                    auto as_date = [](auto &os, const auto &date){
                        os << date;
                    };
                    auto add_field = [&](const auto &value, const auto descr, auto &&streamer){
                        oss_.str(L"");
                        oss_ << std::setw(align) << descr << L": ";
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
                            oss_.str(L"");
                            oss_ << std::setw(align) << descr << L": ";
                            for (const auto &tag: tags)
                                oss_ << tag << L" ";
                            lst->items.push_back(oss_.str());
                        }
                    };
                    add_field(node->value.metadata.dependencies, L"dependencies", as_size);
                    add_field(node->value.metadata.effort(), L"effort", as_number);
                    add_tags(node->value.metadata.tags(), L"tags");

                    add_field_opt(node->value.metadata.my_effort, L"my effort", as_number);
                    add_field_opt(node->value.metadata.my_impact, L"my impact", as_number);
                    add_field_opt(node->value.metadata.my_completion_pct, L"my completion", as_pct);
                    add_field_opt(node->value.metadata.my_volume_db, L"my volume", as_volume);
                    add_field_opt(node->value.metadata.my_live, L"my live", as_date);
                    add_field_opt(node->value.metadata.my_dead, L"my dead", as_date);
                    add_tags(node->value.metadata.my_tags, L"my tags");
                }
                view_.metadata = lst;
            }

            {
                auto lst = dto::List::create();
                lst->name = L"Details";
                if (Commander::state)
                    switch (*Commander::state)
                    {
                        case State::BookmarkRegister:
                        case State::BookmarkJump:
                            lst->name = L"Bookmarks";
                            model_.each_bookmark([&](auto wchar, const auto &path){
                                    oss_.str(L"");
                                    oss_ << wchar << L" => " << model::to_wstring(path);
                                    lst->items.emplace_back(oss_.str());
                                    });
                            break;
                        case State::SetMetadataField:
                        case State::ShowMetadataField:
                            lst->name = L"Metadata fields";
                            for (auto wchar: {L'e', L'v', L'i', L'c', L'l', L'd', L't', L'D'})
                            {
                                oss_.str(L"");
                                oss_ << wchar << L": ";
                                if (auto mf = to_metadata_field(wchar))
                                    oss_ << to_wstring(*mf);
                                lst->items.emplace_back(oss_.str());
                            }
                            break;
                        default: break;
                    }
                view_.details = lst;
            }
        }

        {
            oss_.str(L"");
            if (Commander::state)
                switch (*Commander::state)
                {
                    case State::BookmarkRegister:  oss_ << L"Registering bookmark"; break;
                    case State::BookmarkJump:      oss_ << L"Jump to bookmark"; break;
                    case State::SetMetadataField:
                                                   if (!Commander::metadata_field)
                                                       oss_ << L"Choose metadata field";
                                                   else
                                                       oss_ << L"Metadata for " << to_wstring(*Commander::metadata_field) << L": " << Commander::content;
                                                   break;
                    case State::ShowMetadataField: oss_ << L"Show metadata field"; break;
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

                log::raw([&](auto &os){os << "Before commander_open() " << oss.str() << "\n";});
                std::system(oss.str().c_str());
                log::raw([&](auto &os){os << "After commander_open()\n";});

                std::filesystem::current_path(orig_dir);
                log::ostream() << "Back in " << std::filesystem::current_path() << std::endl;

                return true;
            };
            view_.quit();
        }
    }
    void Presenter::commander_bookmark(wchar_t wchar, bool do_register)
    {
        if (do_register)
            model_.register_bookmark(wchar);
        else
            model_.jump_to_bookmark(wchar);
    }
    void Presenter::commander_set_metadata(MetadataField field, const std::wstring &content)
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
                    else if (content[0] == L'~')
                        dst.reset();
                    else
                        dst = std::stod(content);
                }
                catch (std::invalid_argument) { was_set = false; }
            };
            auto as_date = [&](auto &dst)
            {
                if (content.empty())
                    dst = L"now";
                else if (content[0] == L'~')
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
                                                   else if (content == L"~")
                                                       node->value.metadata.my_tags.clear();
                                                   else if (content[0] == L'~')
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
