#include <sstream>

#include <re2/re2.h>

#include <zap/url.hpp>

namespace zap {

static const re2::RE2 url_re(
    "(?:(\\w+)://)"           // scheme
    "(?:(\\w+)\\:(\\w+)@)?"    // user:password@
    "([^/:]+)"                // hostname
    "(?:\\:(\\d*))?"           // port
    "(.*)"                     // uri
);

url
parse_url(const std::string& s)
{
    url u;

    u.parsed = re2::RE2::FullMatch(
        s,
        url_re,
        &u.scheme,
        &u.user,
        &u.password,
        &u.hostname,
        &u.port,
        &u.uri
    );

    return u;
}

std::string
to_string(const url& u)
{
    std::ostringstream oss;

    oss
        << "url: "
        << " parsed=" << u.parsed
        << " scheme=\"" << u.scheme << "\""
        << " user=\"" << u.user << "\""
        << " password=\"" << u.password << "\""
        << " hostname=\"" << u.hostname << "\""
        << " port=\"" << u.port << "\""
        << " uri=\"" << u.uri << "\""
        ;

    return oss.str();
}

}
