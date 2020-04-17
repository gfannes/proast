#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace model { 

    class Model
    {
    public:
        void set_events_dst(model::Events *events)
        {
            events_ = events;
            if (events_)
                events_->message("Events destination was set");
        }

        //TODO: Replace with enum
        std::string mode;
        void set_mode(const std::string &m)
        {
            std::string new_mode;
            if (false) {}
            else if (m == "input")    {new_mode = m;}
            else if (m == "system")   {new_mode = m;}
            else if (m == "features") {new_mode = m;}
            else if (m == "planning") {new_mode = m;}
            else if (m == "report")   {new_mode = m;}
            if (!new_mode.empty() && new_mode != mode)
            {
                mode = new_mode;
                events_->notify();
            }
        }

        bool operator()()
        {
            MSS_BEGIN(bool);
            MSS_END();
        }

    private:
        model::Events *events_{};
    };

} } 

#endif
