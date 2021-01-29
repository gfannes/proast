#ifndef HEADER_proast_model_Path_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Path_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>

namespace proast { namespace model { 

    using Path = std::vector<std::wstring>;

    std::wstring to_wstring(const Path &);
    Path to_path(const std::wstring &);

    bool pop_if(Path &path, const Path &needle);

} } 

#endif
