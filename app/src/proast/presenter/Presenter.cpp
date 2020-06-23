#include <proast/presenter/Presenter.hpp>

namespace proast { namespace presenter { 

    std::string hr(DisplayAttribute displayattribute)
    {
        switch (displayattribute)
        {
            case DisplayAttribute::My: return "My";
            case DisplayAttribute::Total: return "Total";
            case DisplayAttribute::Done: return "Done";
            case DisplayAttribute::Todo: return "Todo";
            case DisplayAttribute::Deadline: return "Deadline";
            case DisplayAttribute::ETA: return "ETA";
        }
        return "<Unknown DisplayAttribute>";
    }

} } 
