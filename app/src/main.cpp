#include <proast/App.hpp>
#include <gubg/mss.hpp>
#include <iostream>

namespace proast { 
    bool main(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        proast::App app;

        MSS(app.options.parse(argc, argv));
        app.options.roots.emplace_back("/home/geertf/proast");
        app.options.roots.emplace_back("/home/geertf/gubg");
        app.options.roots.emplace_back("/home/geertf/subsoil");
        app.options.roots.emplace_back("/home/geertf/nontech");
        app.options.roots.emplace_back("/home/geertf/decode-it/momu");
        app.options.roots.emplace_back("/home/geertf/decode-it/auro");
        /* app.options.roots.emplace_back("/home/geertf/auro/all"); */
        if (app.options.verbose_level >= 1)
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
