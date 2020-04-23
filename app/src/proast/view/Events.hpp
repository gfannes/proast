#ifndef HEADER_proast_view_Events_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Events_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { namespace view { 

    //Events emitted by the View, meant to be handled by the Presenter

    class Events
    {
    public:
        virtual ~Events() {}

        void message(const std::string &str) {return view_message(str);}
        void received(const char32_t ch) {return view_received(ch);}

    protected:
        virtual void view_message(const std::string &str) = 0;
        virtual void view_received(const char32_t ch) = 0;
    };

} } 

#endif
