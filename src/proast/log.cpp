#include <proast/log.hpp>
#include <sstream>

namespace proast { namespace log { 

    namespace my { 
        std::ostringstream &oss()
        {
            static std::ostringstream s_oss;
            return s_oss;
        }
    } 

    std::ostream &stream()
    {
        return my::oss();
    }

    std::string content()
    {
        return my::oss().str();
    }

} } 
