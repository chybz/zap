#pragma once

#include <vector>
#include <variant>

#include <zap/types.hpp>
#include <zap/repository.hpp>

namespace zap {

namespace remotes {

struct git
{
    std::string scheme;
    std::string netloc;
    std::string author;
    std::string name;
    std::string ref;
};

struct github : git
{};

struct gitlab : git
{};

struct direct
{
    std::string url;
    std::string name;
    std::string version;
};

} // namespace remotes

using remote = std::variant<
    remotes::github,
    remotes::gitlab,
    remotes::direct
>;

std::string
remote_to_string(const remote& r);

remote
remote_from_string(const std::string& s);

struct dependency
{
    remote r;
    strings_map opts;

    std::string to_string() const;
};

using dependencies = std::vector<dependency>;

}
