#ifndef HEADER_proast_dto_StyleString_hpp_ALREADY_INCLUDED
#define HEADER_proast_dto_StyleString_hpp_ALREADY_INCLUDED

#include <string>
#include <map>

namespace proast { namespace dto { 
    class StyleString
    {
    public:
        std::string str;
        std::map<unsigned int, int> ix__attention;
        std::map<unsigned int, bool> ix__bold;

        void operator=(const std::string &str) {this->str = str;}

        int attention(unsigned int ix) const;
        bool bold(unsigned int ix) const ;

    private:
    };
} } 

#endif
