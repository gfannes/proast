#ifndef HEADER_proast_model_Events_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Events_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { namespace model { 

    //Events emitted by the Model, meant to be handled by the Presenter

    class Events
    {
    public:
        virtual ~Events() {}

        void message(const std::string &str) {return model_message(str);}
        void notify(){return model_notify();}

    protected:
        virtual void model_message(const std::string &str) = 0;
        virtual void model_notify() = 0;
    };

} } 

#endif
