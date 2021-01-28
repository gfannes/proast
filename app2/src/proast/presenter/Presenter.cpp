#include <proast/presenter/Presenter.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace proast { namespace presenter { 
    Presenter::Presenter(model::Model &model, view::View &view): model_(model), view_(view)
    {
        view_.events = this;
    }

    bool Presenter::run()
    {
        MSS_BEGIN(bool);

        MSS(refresh_view_());

        view_.run();

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
            auto set_view_dto = [&](auto &dst, auto node)
            {
                if (!node)
                {
                    dst.reset();
                    return;
                }
                if (std::filesystem::is_regular_file(node->value.path))
                {
                    if (!node->value.content)
                        node->value.content = content_.create(node->value.path);
                    dst = node->value.content;
                }
                else
                {
                    dst = dto::List::create();
                    for (auto &n: node->childs.nodes)
                        dst->items.emplace_back(n.value.name);
                    dst->ix = model::Tree::selected_ix(*node);
                }
            };
            set_view_dto(view_.n0, model_.node_0());
            set_view_dto(view_.n0a, model_.node_0a());
            set_view_dto(view_.n00, model_.node_00());
            set_view_dto(view_.n0b, model_.node_0b());
            set_view_dto(view_.n00a, model_.node_00a());
            set_view_dto(view_.n000, model_.node_000());
            set_view_dto(view_.n00b, model_.node_00b());
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
        refresh_view_();
    }
    void Presenter::commander_open(bool edit)
    {
        if (auto me = model_.node_000())
        {
            const auto path = me->value.path;
            const auto orig_dir = std::filesystem::current_path();
            std::filesystem::current_path(path.parent_path());

            std::ostringstream oss;
            if (edit)
                oss << "nvim " << path.filename();
            else
                oss << "bash";

            log::raw([&](auto &os){os << "Before commander_open(" << edit << ") " << oss.str() << "\n";});
            std::system(oss.str().c_str());
            log::raw([&](auto &os){os << "After commander_open(" << edit << ")\n";});

            std::filesystem::current_path(orig_dir);
        }
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        log::raw([=](auto &os){
                os << "Received event " << (unsigned int)wchar;
                if (wchar < 128) os << " '" << (char)wchar << "'";
                
                os << std::endl;});
        Commander::process(wchar);
    }
} } 
