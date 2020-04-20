#include <proast/version.hpp>

#ifdef PROAST_GIT_HASH
#error PROAST_GIT_HASH alroady defined
#endif
#include <proast/version_git_hash.hpp>

namespace proast { namespace version { 

    std::string git_hash()
    {
#ifdef PROAST_GIT_HASH
        return PROAST_GIT_HASH;
#else
        return "<invalid git hash>";
#endif
    }

} } 
