#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/Commander.hpp>
#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>
#include <sstream>

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
        void commander_move(Direction, int level = 0);

    private:
        using Commander = Commander_crtp<Presenter>;

        bool refresh_view_();

        model::Model &model_;
        view::View &view_;

        mutable std::basic_ostringstream<wchar_t> oss_;
    };
} } 

#endif
