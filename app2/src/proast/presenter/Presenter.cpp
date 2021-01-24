#include <proast/presenter/Presenter.hpp>

namespace proast { namespace presenter { 
    void Presenter::run()
    {
        view_.run();
    }

    //View::Events
    void Presenter::received(wchar_t wchar)
    {
        switch (wchar)
        {
            case L'q':
                view_.quit();
                break;
        }
    }
} } 
