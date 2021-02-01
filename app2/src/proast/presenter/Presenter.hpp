#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/Commander.hpp>
#include <proast/model/Model.hpp>
#include <proast/model/Content.hpp>
#include <proast/view/View.hpp>
#include <sstream>
#include <functional>

namespace proast { namespace presenter { 
    class Presenter: public view::View::Events, public Commander_crtp<Presenter>
    {
    public:
        Presenter(model::Model &model, view::View &view);

        bool run();

        //View::Events API
        void received(wchar_t) override;

        //Commander API
        void commander_quit();
        void commander_move(Direction, bool me);
        void commander_open(Open);
        void commander_bookmark(wchar_t wchar, bool do_register);

    private:
        using Commander = Commander_crtp<Presenter>;

        bool refresh_view_();

        model::Model &model_;
        view::View &view_;

        model::Content content_;

        std::function<bool()> scheduled_operation_;

        mutable std::basic_ostringstream<wchar_t> oss_;
    };
} } 

#endif
