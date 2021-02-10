#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/presenter/Presenter.hpp>
#include <iostream>
#include <memory>

namespace proast { 
    class App
    {
    public:
        Options options;

        bool run();

    private:
        model::Model model_;
        view::View view_;
        presenter::Presenter presenter_{model_, view_};
    };
} 

#endif
