#ifndef HEADER_proast_model_Markdown_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Markdown_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <proast/model/Config.hpp>
#include <filesystem>

namespace proast { namespace model { namespace markdown { 

    bool read_string(Node &node, const std::string &markdown);
    bool read_directory(Node &node, const std::filesystem::path &directory, const Config &config);

    bool write_string(std::string &markdown, const Node &);
    bool write_directory(const std::filesystem::path &directory, const Node &node, const Config &config);

} } } 

#endif
