#include <iostream>

#include <zap/cmdline.hpp>
#include <zap/utils.hpp>
#include <zap/env.hpp>
#include <zap/cmake/trace_parser.hpp>

int main(int ac, char** av)
{
    zap::cmake::trace_parser tp;

    auto p = tp.parse(av[1], av[2]);

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
