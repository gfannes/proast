#include <proast/model/Path.hpp>
#include <gubg/Strange.hpp>
#include <sstream>

namespace proast { namespace model { 

    std::string to_string(const Path &path)
    {
        std::ostringstream oss;
        for (const auto &e: path)
            oss << '/' << e;
        return oss.str();
    }
    Path to_path(const std::string &str)
    {
        Path path;
        gubg::Strange strange{str};
        strange.pop_if('/');
        for (std::string part; !strange.empty(); path.push_back(part))
            strange.pop_until(part, '/') || strange.pop_all(part);
        return path;
    }

} } 
