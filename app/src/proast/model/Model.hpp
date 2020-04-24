#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/ListBox.hpp>
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

        ListBox mode_lb = {"develop", "rework"};
        void set_mode(const std::string &m)
        {
            if (mode_lb.set_active(m))
                events_->notify();
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
