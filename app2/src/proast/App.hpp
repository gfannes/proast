#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/presenter/Presenter.hpp>
#include <proast/ui/List.hpp>
#include <iostream>
#include <memory>

namespace proast { 
    class App
    {
    public:
        bool parse(int argc, const char **argv);

        bool run();

    private:
        Options options_;
        model::Model model_;
        view::View view_;
        presenter::Presenter presenter_{model_, view_};
    };
} 

#endif
