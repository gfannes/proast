#include <proast/App.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <iostream>

bool main_(int argc, const char **argv)
{
    MSS_BEGIN(bool);
    proast::App app;
    for (bool quit = false; !quit;)
    {
        MSS(app.process(quit));
    }
    MSS_END();
}

int main(int argc, const char **argv)
{
    MSS_BEGIN(int);
    const auto ok = main_(argc, argv);
    std::cout << proast::log::content() << std::endl;
    MSS(ok);
    MSS_END();
}
