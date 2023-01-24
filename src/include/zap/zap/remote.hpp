#pragma once

#include <string>
#include <unordered_map>

namespace zap {

enum class remote_type
{
    none,
    github,
    gitlab,
    bitbucket
};

using remote_to_string_map = std::unordered_map<remote_type, std::string>;
using string_to_remote_map = std::unordered_map<std::string, remote_type>;

const remote_to_string_map&
get_remote_to_string_map();

const string_to_remote_map&
get_string_to_remote_map();

const std::string&
to_string(remote_type t);

remote_type
from_string(const std::string& s);

bool
valid_remote(const std::string& s);

void
check_remote(const std::string& s);

}
