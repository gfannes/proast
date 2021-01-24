#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>

namespace proast { namespace presenter { 
    class Presenter: public view::View::Events
    {
    public:
        Presenter(model::Model &model, view::View &view): model_(model), view_(view)
        {
            view_.events = this;
        }

        void run();

        //View::Events
        void received(wchar_t) override;

    private:

        model::Model &model_;
        view::View &view_;
    };
} } 

#endif
