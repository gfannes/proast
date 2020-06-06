#ifndef HEADER_proast_model_Item_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Item_hpp_ALREADY_INCLUDED

#include <proast/model/Path.hpp>
#include <gubg/markup/Document.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace proast { namespace model { 

    //TODO: Replace this with a real struct
    //For now, std::string was chosen because this can be compared when using the format YYYYMMDD
    using DateTime = std::string;

    enum class Status
    {
        Todo, InDesign, Designed, Implementing, Implemented, Done,
    };
    std::string hr(Status);

    enum class Priority
    {
        Must, Should, Could, Wont,
    };
    std::string hr(Priority);

    enum class Type
    {
        Feature, Requirement, Design, Deliverable, Free,
    };
    std::string hr(Type);

    enum class Style
    {
        Title, Section, Bullet, Margin,
    };
    std::string hr(Style);

    class Item
    {
    public:
        std::string key() const;
        void set_key(const std::string &);
        void set_key(Type, const std::string &);
        void set_key(Type, unsigned int ix);

        std::string stem() const;

        DateTime deadline;
        std::optional<double> my_cost;
        std::optional<Style> style;
        Status status = Status::Todo;
        std::optional<Path> link;
        std::optional<Priority> priority;

        std::optional<std::filesystem::path> content_fp;

        const std::string &title() const;
        std::string key_as_title() const;
        void set_title(const std::string &);

        std::vector<std::string> description;

        Type type() const;

        //Indicates if this item is embedded in the content of its parent
        bool is_embedded() const;

        void clear() {*this = Item{};}

        //Members to remove

        //Replace this with markdown::ast::Tree
        gubg::markup::Document preview;
        std::string active_child_key;
        std::optional<std::filesystem::path> directory;

        void stream(std::ostream &os) const;

    private:
        std::string key_;
        Type type_ = Type::Free;
        std::string title_;
    };

    inline std::ostream &operator<<(std::ostream &os, const Item &item)
    {
        item.stream(os);
        return os;
    }

} } 

#endif
