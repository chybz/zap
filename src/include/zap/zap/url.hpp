#pragma once

namespace zap {

struct url
{
    bool parsed = false;
    std::string scheme;
    std::string user;
    std::string password;
    std::string hostname;
    std::string port;
    std::string uri;

    url();
    url(const std::string& s);

    std::string host() const;

    bool parse(const std::string& s);
    std::string to_string() const;
};

}
