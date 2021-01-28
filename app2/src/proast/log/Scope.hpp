#ifndef HEADER_proast_log_Scope_hpp_ALREADY_INCLUDED
#define HEADER_proast_log_Scope_hpp_ALREADY_INCLUDED

#include <proast/log/raw.hpp>

namespace proast { namespace log { 
    class Scope
    {
    public:
        class Header
        {
        public:
            Header(std::ostream &os): os_(os) {}
            template <typename Key, typename Value>
            Header &attr(const Key &key, const Value &value)
            {
                os_ << "(" << key << ":" << value << ")";
                return *this;
            }
            template <typename Key>
            Header &attr(const Key &key)
            {
                os_ << "(" << key << ")";
                return *this;
            }
        private:
            std::ostream &os_;
        };

        Scope(const std::string &name): Scope(name, true) {}
        template <typename Ftor>
        Scope(const std::string &name, Ftor &&ftor): Scope(name, false)
        {
            Header h{ostream()};
            ftor(h);
            ostream() << "{" << std::endl;
        }
        ~Scope();
        
        template <typename Ftor>
        void line(Ftor &&ftor)
        {
            ostream() << std::string(level__*2, ' ');
            ftor(ostream());
            ostream() << std::endl;
        }

    private:
        Scope(const std::string &name, bool close_header);
        static unsigned int level__;
    };
} } 

#endif
