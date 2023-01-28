#include <iostream>
#include <stdexcept>

#include <zap/cmdline.hpp>
#include <zap/url.hpp>
#include <zap/log.hpp>

void catch_global()
{
    auto eptr = std::current_exception();

    if (eptr) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::cerr
                << "E: "
                << e.what()
                << std::endl
                ;
        } catch (...) {
            std::cerr << "exiting due to unhandled exception" << std::endl;
        }
    } else {
        std::cerr << "exiting due to unhandled error" << std::endl;
    }

    exit(1);
}

int main(int ac, char** av)
{
    auto u = zap::parse_url(av[1]);

    zap::die_unless(u.parsed, "invalid url: ", av[1]);

    std::cout << zap::to_string(u) << std::endl;

    return 0;

    std::set_terminate(catch_global);

    int rc = 0;

    auto cl = zap::parse(ac, av);

    if (cl.exit) {
        rc = 1;
    } else {
        cl.run();
    }

    return rc;
}
