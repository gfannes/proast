#include <proast/App.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/mss.hpp>
#include <list>

namespace proast { 
    bool App::run()
    {
        MSS_BEGIN(bool);

        if (options.print_help)
        {
            std::cout << options.help();
        }
        else if (!options.command.empty())
        {
            gubg::naft::Document doc{std::cout};
            unsigned int ok_count = 0;
            std::list<std::filesystem::path> fails;
            for (const auto &root: options.roots)
            {
                {
                    std::filesystem::current_path(root);
                    auto run_node = doc.node("Run");
                    run_node.attr("command", options.command);
                    run_node.attr("pwd", std::filesystem::current_path().string());
                    run_node.text("\n");

                    std::flush(std::cout);
                    const auto rc = std::system(options.command.c_str());
                    {
                        auto status_node = run_node.node("Status"); 
                        status_node.attr("rc", rc);
                        if (rc == 0)
                            ++ok_count;
                        else
                            fails.push_back(std::filesystem::current_path());
                    }
                }
                doc.text("\n");
            }
            auto status_node = doc.node("Status");
            status_node.attr("fail", fails.size());
            status_node.attr("ok", ok_count);
            for (auto fp: fails)
            {
                auto fail_node = status_node.node("Fail");
                fail_node.attr("path", fp.string());
            }
        }
        else
        {
            MSS(model_.set_home_dir(options.home_dir));

            proast::model::Model::Config config;
            for (const auto &root: options.roots)
            {
                if (options.verbose_level >= 0)
                    std::cout << "Loading " << root << std::endl;
                if (!model_.add_root(root, config))
                    std::cout << "Error: could not add " << root << std::endl;
            }

            MSS(presenter_.run());
        }

        MSS_END();
    }
} 
