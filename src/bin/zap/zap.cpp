#include <iostream>

#include <zap/cmdline.hpp>
#include <zap/utils.hpp>

int main(int ac, char** av)
{
    int rc = 0;

    auto cl = zap::parse(ac, av);

    if (cl.exit) {
        rc = 1;
    } else {
        cl.run();
    }

    return rc;
}
