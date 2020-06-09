#include <proast/presenter/Commander.hpp>

namespace proast { namespace presenter { 

    std::string hr(State state)
    {
        switch (state)
        {
            case State::Idle: return "Idle";
            case State::Add: return "Add";
            case State::Rename: return "Rename";
            case State::Remove: return "Remove";
            case State::Cost: return "Cost";
            case State::RegisterBookmark: return "RegisterBookmark";
            case State::LoadBookmark: return "LoadBookmark";
            case State::SetType: return "SetType";
            case State::SetState: return "SetState";
            default: break;
        }
        return "<Unknown State>";
    }

} } 
