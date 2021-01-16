#include <proast/App.hpp>
#include <gubg/mss.hpp>
#include <iostream>

namespace proast { 
    int main(int argc, const char **argv)
    {
        MSS_BEGIN(int);

        proast::App app;
        MSS(app.parse(argc, argv));

        MSS(app.prepare());

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
