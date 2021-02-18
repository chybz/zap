#include <iostream>

#include <zap/cmdline.hpp>
#include <zap/utils.hpp>
#include <zap/toolchain.hpp>
#include <zap/cmake/trace_parser.hpp>

int main(int ac, char** av)
{
    zap::cmake::trace_parser tp(av[1]);

    // int rc = 0;

    // auto cl = zap::parse(ac, av);

    // if (cl.exit) {
    //     rc = 1;
    // } else {
    //     cl.run();
    // }
    //
    // return rc;

    return 0;
}
