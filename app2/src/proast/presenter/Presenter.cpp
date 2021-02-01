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

        {
            oss_.str(L"");
            oss_ << model_.node()->value.path.wstring();
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
                        node->value.content = content_.create(node->value.path);
                    lst = node->value.content;
                }
                else
                {
                    lst = dto::List::create();
                    for (auto &n: node->childs.nodes)
                        lst->items.emplace_back(n.value.name);
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
                lst->name = L"Metadata";
                view_.metadata = lst;
            }

            {
                auto lst = dto::List::create();
                lst->name = L"Details";
                switch (state())
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
                    default: break;
                }
                view_.details = lst;
            }
        }
        
        {
            std::wstring wstr;
            switch (state())
            {
                case State::BookmarkRegister: wstr = L"Registering bookmark"; break;
                case State::BookmarkJump:     wstr = L"Jump to bookmark"; break;
                default: break;
            }
            view_.footer = wstr;
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
    void Presenter::commander_reload()
    {
        model_.reload();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        log::raw([=](auto &os){
                os << "Received event " << (unsigned int)wchar;
                if (wchar < 128) os << " '" << (char)wchar << "'";
                
                os << std::endl;});
        Commander::process(wchar);
        refresh_view_();
    }
} } 
