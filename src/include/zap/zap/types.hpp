#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <zap/list_map.hpp>

namespace zap {

using strings = std::vector<std::string>;
using string_set = std::unordered_set<std::string>;
using string_map = std::unordered_map<std::string, std::string>;
using string_list_map = list_map<std::string, std::string>;

using string_views = std::vector<std::string_view>;
using string_view_set = std::unordered_set<std::string_view>;

}
