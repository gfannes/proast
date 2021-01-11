#include <proast/Options.hpp>
#include <proast/Tree.hpp>
#include <gubg/mss.hpp>
#include <iostream>

namespace proast { 
    int main(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        proast::Options options;
        MSS(options.parse(argc, argv));
        options.stream(std::cout);

        proast::Tree tree;
        proast::Tree::Config config;
        for (const auto &root: options.roots)
        {
            MSS(tree.add(root, config));
        }

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
