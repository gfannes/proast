#include <proast/presenter/Presenter.hpp>
#include <gubg/mss.hpp>

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

        auto path = model_.current_path();

        {
            oss_.str(L"");
            model::Tree::Datas datas;
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
            MSS(model_.tree.resolve_nodes(nodes, path));
            const auto node_cnt = nodes.size();
            if (node_cnt >= 2)
            {
                auto parent = nodes[node_cnt-2];

                n00_ = dto::List::create();
                for (auto &n: parent->childs.nodes)
                    n00_->items.emplace_back(n.value.name);
                n00_->ix = model::Tree::selected_ix(*parent);
                view_.n00 = n00_;
            }
        }

        MSS_END();
    }

    //Commander API
    void Presenter::commander_quit()
    {
        view_.quit();
    }
    void Presenter::commander_move(Direction direction)
    {
        model_.move(direction);
        refresh_view_();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        Commander::process(wchar);
    }
} } 
