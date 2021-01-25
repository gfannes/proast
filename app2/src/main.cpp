#include <proast/App.hpp>
#include <gubg/mss.hpp>
#include <iostream>

namespace proast { 
    bool main(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        proast::App app;

        MSS(app.options.parse(argc, argv));
        app.options.roots.emplace_back(std::filesystem::current_path());
        /* app.options.roots.emplace_back("/home/geertf/gubg"); */
        if (app.options.verbose >= 1)
            app.options.stream(std::cout);

        MSS(app.run());

        MSS_END();
    }
} 

int main(int argc, const char **argv)
{
    if (!proast::main(argc, argv))
    {
        std::cout << "Error: something went wrong" << std::endl;
        return -1;
    }
    std::cout << "Everything went OK" << std::endl;
    return 0;
}
