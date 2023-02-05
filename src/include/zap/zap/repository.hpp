#pragma once

#include <string>
#include <unordered_map>

namespace zap {

enum class repository_type
{
    none,
    github,
    gitlab,
    bitbucket
};

using repository_to_string_map = std::unordered_map<
    repository_type, std::string
>;

using string_to_repository_map = std::unordered_map<
    std::string, repository_type
>;

const repository_to_string_map&
get_repository_to_string_map();

const string_to_repository_map&
get_string_to_repository_map();

const std::string&
to_string(repository_type t);

repository_type
to_repository(const std::string& s);

bool
valid_repository(const std::string& s);

void
check_repository(const std::string& s);

}
