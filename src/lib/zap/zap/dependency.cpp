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
                        r.id,
                        "archive/refs/tags",
                        join("-", r.name, r.version) + ".zip"
                    )
                    ;
            },
            [&oss](const remotes::gitlab& r) {
                // https://gitlab.expandium.com/mno/himport/-/archive/v2022.Q1.3/himport-v2022.Q1.3.zip
                oss
                    << remote_host_or(r, "https://gitlab.com")
                    << join(
                        "/",
                        r.id,
                        "-/archive",
                        r.version,
                        join("-", r.name, r.version) + ".zip"
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
