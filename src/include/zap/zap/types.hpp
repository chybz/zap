#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <zap/vector_map.hpp>

namespace zap {

using strings = std::vector<std::string>;
using strings_map = std::unordered_map<std::string, strings>;
using string_set = std::unordered_set<std::string>;
using string_set_map = std::unordered_map<std::string, string_set>;
using string_map = std::unordered_map<std::string, std::string>;
using string_vector_map = vector_map<std::string, std::string>;

using string_views = std::vector<std::string_view>;
using string_view_set = std::unordered_set<std::string_view>;

}
