#include <iostream>

#include <zap/cmdline.hpp>
#include <zap/utils.hpp>
#include <zap/toolchain.hpp>
#include <zap/cmake_configs.hpp>

int main(int ac, char** av)
{
    int rc = 0;

    const auto& tc = zap::get_toolchain();

    auto cl = zap::parse(ac, av);

    if (cl.exit) {
        rc = 1;
    } else {
        cl.run();
    }

    return rc;
}
