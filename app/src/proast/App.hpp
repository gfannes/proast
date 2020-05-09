#ifndef HEADER_proast_App_hpp_ALREADY_INCLUDED
#define HEADER_proast_App_hpp_ALREADY_INCLUDED

#include <proast/Options.hpp>
#include <proast/version.hpp>
#include <proast/log.hpp>
#include <proast/presenter/Presenter.hpp>
#include <gubg/mss.hpp>
#include <optional>
#include <thread>
#include <memory>

namespace proast { 
    class App
    {
    public:
        bool process(int argc, const char **argv)
        {
            return options_.parse(argc, argv);
        }

        bool prepare()
        {
            MSS_BEGIN(bool);

            bool create_presenter = true;
            if (options_.print_help)
            {
                std::cout << options_.help();
                create_presenter = false;
            }
            if (options_.print_version)
            {
                std::cout << "Version: " << version::major() << "." << version::minor() << "." << version::patch() << ", git hash: " << version::git_hash() << std::endl;
                create_presenter = false;
            }

            if (create_presenter)
            {
                view_.reset(new view::View);
                presenter_.reset(new presenter::Presenter{model_, *view_});
            }

            MSS_END();
        }

        bool quit() const {return !presenter_;}
        bool operator()()
        {
            MSS_BEGIN(bool);

            MSS(!!view_);
            auto &view = *view_;
            MSS(!!presenter_);
            auto &presenter = *presenter_;

            MSS(model_());
            MSS(view());
            MSS(presenter());

            if (presenter.quit)
            {
                log::stream() << "Quitting" << std::endl;
                presenter_.reset();
            }

            //Brute-force manner to reduce CPU burning
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            MSS_END();
        }

    private:
        Options options_;

        model::Model model_;
        std::shared_ptr<view::View> view_;
        std::shared_ptr<presenter::Presenter> presenter_;
    };
} 

#endif
