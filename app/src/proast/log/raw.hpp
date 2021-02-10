#ifndef HEADER_proast_log_raw_hpp_ALREADY_INCLUDED
#define HEADER_proast_log_raw_hpp_ALREADY_INCLUDED

#include <ostream>

namespace proast { namespace log { 
    std::ostream &ostream();

    template <typename Ftor>
    void raw(Ftor &&ftor)
    {
        auto &os = ostream();
        ftor(os);
        std::flush(os);
    }
} } 

#endif
