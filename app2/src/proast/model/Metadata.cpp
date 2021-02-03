#include <proast/model/Metadata.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace model { 
    void Metadata::reset_aggregated_data()
    {
        effort = 0.0;
        tags.clear();
    }
} } 
