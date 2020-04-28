#ifndef HEADER_proast_model_Path_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Path_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <cassert>

namespace proast { namespace model { 

    class Path
    {
    public:
        template <typename Ftor>
        void each_segment(Ftor &&ftor) const
        {
            assert(size_ <= elements_.size());
            for (auto ix = 0u; ix < size_; ++ix)
                ftor(elements_[ix]);
        }

    private:
        std::vector<std::string> elements_;
        std::size_t size_ = 0;
    };

} } 

#endif
