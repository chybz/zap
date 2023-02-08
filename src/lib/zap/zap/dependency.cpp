#include <sstream>

#include <zap/dependency.hpp>
#include <zap/utils.hpp>
#include <zap/variant_utils.hpp>
#include <zap/join_utils.hpp>
#include <zap/url.hpp>
#include <zap/log.hpp>

namespace zap {

std::string
remote_host_or(const remotes::git& r, const std::string& alt)
{ return r.netloc.empty() ? alt : join("://", r.scheme, r.netloc); }

std::string
remote_to_string(const remote& r)
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

remote
to_remote(
    repository_type type,
    const std::string& base,
    const std::string& spec,
    const std::string& ref
)
{
    remote r;
    url u(base);
    auto parts = split("/", spec);

    die_unless(u.parsed, "invalid remote base: ", base);
    die_unless(parts.size() == 2, "unknown remote spec: ", spec);

    switch (type) {
        case repository_type::github:
        r = remotes::github{{
            .scheme = u.scheme,
            .netloc = u.scheme
        }};
        break;
        case repository_type::gitlab:
        break;
        case repository_type::bitbucket:
        break;
        case repository_type::none:
        break;
    }

    return r;
}

std::string
dependency::to_string() const
{
    std::ostringstream oss;

    oss
        << "url: " << remote_to_string(r)
        // TODO: dump opts
        ;

    return oss.str();
}

}
