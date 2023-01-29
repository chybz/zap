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

url::url()
{}

url::url(const std::string& s)
{ parse(s); }

std::string
url::host() const
{
    std::ostringstream oss;

    if (!scheme.empty()) {
        oss << scheme << "://";
    }

    if (!user.empty()) {
        oss << user;

        if (!password.empty()) {
            oss << ":" << password;
        }

        oss << "@";
    }

    oss << hostname;

    if (!port.empty()) {
        oss << ":" << port;
    }

    return oss.str();
}

bool
url::parse(const std::string& s)
{
    parsed = re2::RE2::FullMatch(
        s,
        url_re,
        &scheme,
        &user,
        &password,
        &hostname,
        &port,
        &uri
    );

    return parsed;
}

std::string
url::to_string() const
{
    std::ostringstream oss;

    oss
        << "url: "
        << " parsed=" << parsed
        << " scheme=\"" << scheme << "\""
        << " user=\"" << user << "\""
        << " password=\"" << password << "\""
        << " hostname=\"" << hostname << "\""
        << " port=\"" << port << "\""
        << " uri=\"" << uri << "\""
        ;

    return oss.str();
}

}
