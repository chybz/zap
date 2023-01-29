#include <sstream>

#include <zap/dependency.hpp>
#include <zap/variant_utils.hpp>
#include <zap/join_utils.hpp>

namespace zap {

std::string
remote_host_or(const remotes::git& r, const std::string& alt)
{ return r.netloc.empty() ? alt : join("://", r.scheme, r.netloc); }

std::string
remote_url(const remote& r)
{
    std::ostringstream oss;

    std::visit(
        overloaded{
            [&oss](const remotes::github& r) {
                oss
                    << remote_host_or(r, "https://github.com")
                    << join(
                        "/",
                        r.author,
                        "archive/refs/tags",
                        join("-", r.name, r.ref) + ".zip"
                    )
                    ;
            },
            [&oss](const remotes::gitlab& r) {
                oss
                    << remote_host_or(r, "https://gitlab.com")
                    << join(
                        "/",
                        r.author,
                        "-/archive",
                        r.ref,
                        join("-", r.name, r.ref) + ".zip"
                    )
                    ;
            },
            [&oss](const remotes::direct& r) {
                oss << r.url;
            }
        },
        r
    );

    return oss.str();
}

std::string
dependency::to_string() const
{
    std::ostringstream oss;

    oss
        << "url: " << remote_url(r)
        // TODO: dump opts
        ;

    return oss.str();
}

}
