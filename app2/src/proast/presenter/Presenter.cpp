#include <proast/presenter/Presenter.hpp>
#include <proast/util.hpp>
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
            model::Tree::Datas datas;
            const auto &path = model_.current_path();
            MSS(model_.tree.resolve_datas(datas, path));
            oss_ << L"Current path: ";
            for (auto ptr: datas)
                oss_ << L"/" << ptr->name;
            for (auto ix = 0u; ix < path.size(); ++ix)
                oss_ << L" " << std::to_wstring(path[ix]);
            oss_ << L" " << model_.current_me()->value.path;
            view_.header = oss_.str();
        }

        {
            model::Tree::Nodes nodes;

            auto set_view_dto = [&](auto &dst, model::Path path, int offset)
            {
                dst = dto::List::create();

                if (path.empty())
                    return;

                path.back() += offset;
                if (!model_.tree.resolve_nodes(nodes, path))
                    return;

                auto node = nodes.back();

                if (node->is_leaf() && std::filesystem::is_regular_file(node->value.path))
                {
                    std::ifstream fi{node->value.path};
                    for (std::string line; std::getline(fi, line);)
                        dst->items.emplace_back(to_wstring(line));
                    dst->ix = 0;
                }
                else
                {
                    for (auto &n: node->childs.nodes)
                        dst->items.emplace_back(n.value.name);
                    dst->ix = model::Tree::selected_ix(*node);
                }
            };

            auto path = model_.current_path();
            set_view_dto(view_.n00a, path, -1);
            set_view_dto(view_.n000, path, 0);
            set_view_dto(view_.n00b, path, 1);
            if (!path.empty())
            {
                path.pop_back();
                set_view_dto(view_.n0a, path, -1);
                set_view_dto(view_.n00, path, 0);
                set_view_dto(view_.n0b, path, 1);
                if (!path.empty())
                {
                    path.pop_back();
                    set_view_dto(view_.n0, path, 0);
                }
            }
        }

        MSS_END();
    }

    //Commander API
    void Presenter::commander_quit()
    {
        view_.quit();
    }
    void Presenter::commander_move(Direction direction, int level)
    {
        model_.move(direction, level);
        refresh_view_();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        Commander::process(wchar);
    }
} } 
