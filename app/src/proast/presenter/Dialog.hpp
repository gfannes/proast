#ifndef HEADER_proast_presenter_Dialog_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Dialog_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { namespace presenter { 

    class Dialog
    {
    public:
        void clear() {*this = Dialog{};}

        const std::string &caption() const {return caption_;}
        const std::string &content() const {return content_;}

        void set_caption(const std::string &str) {caption_ = str;}
        void set_content(const std::string &str) {content_ = str;}

    private:
        std::string caption_;
        std::string content_;
    };

} } 

#endif
