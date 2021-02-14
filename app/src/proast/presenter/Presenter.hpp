#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/Commander.hpp>
#include <proast/model/Model.hpp>
#include <proast/model/ContentMgr.hpp>
#include <proast/view/View.hpp>
#include <sstream>
#include <functional>

namespace proast { namespace presenter { 
    class Presenter: public view::View::Events, public Commander_crtp<Presenter>
    {
    public:
        using Commander = Commander_crtp<Presenter>;

        Presenter(model::Model &model, view::View &view);

        bool run();

        //View::Events API
        void received(wchar_t, bool alt) override;

        //Commander API
        void commander_quit();
        void commander_move(Direction, bool me, bool move_node);
        void commander_open(Open, std::string cmd = {});
        void commander_bookmark(char ch, bool do_register);
        void commander_set_metadata(MetadataField, const std::string &content);
        void commander_show_metadata(std::optional<MetadataField>);
        void commander_create(const std::string &name, bool create_file, bool in_parent);
        void commander_rename(const std::string &name);
        void commander_export(const std::string &name);
        void commander_duplicate(const std::string &name);
        void commander_search(const std::string &pattern, bool in_content);
        void commander_delete(Delete);
        void commander_paste(bool paste_in);
        void commander_set_node_state(std::optional<model::State>, bool done);
        void commander_set_order(bool order_sequential);
        void commander_reload();

    private:
        bool refresh_view_();

        model::Model &model_;
        std::optional<MetadataField> show_metadata_field_;
        view::View &view_;

        model::ContentMgr content_mgr_;

        std::function<bool()> scheduled_operation_;

        mutable std::ostringstream oss_;
    };
} } 

#endif
