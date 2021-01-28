#include <proast/dto/List.hpp>
#include <filesystem>
#include <set>
#include <string>

namespace proast { namespace model {
    class Content
    { 
    public:
        Content();

        dto::List::Ptr create(const std::filesystem::path &path) const;

    private:
        std::set<std::string> allowed_extensions_;
    };
} } 
