#include <proast/presenter/Presenter.hpp>

namespace proast { namespace presenter { 

    std::string hr(DisplayCost displaycost)
    {
        switch (displaycost)
        {
            case DisplayCost::My: return "My";
            case DisplayCost::Total: return "Total";
            case DisplayCost::Done: return "Done";
            case DisplayCost::Todo: return "Todo";
        }
        return "<Unknown DisplayCost>";
    }

} } 
