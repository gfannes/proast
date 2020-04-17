#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/log.hpp>
#include <proast/presenter/Presenter.hpp>
#include <gubg/mss.hpp>
#include <ncpp.hh>
#include <optional>
#include <thread>

namespace proast { 
    class App
    {
    public:
        bool process(bool &quit)
        {
            MSS_BEGIN(bool);

            MSS(model_());
            MSS(view_());
            MSS(presenter_());

            quit = presenter_.quit;

            MSS_END();
        }

    private:
        model::Model model_;
        view::View view_;
        presenter::Presenter presenter_{model_, view_};
    };
} 

#endif
