#pragma once

#include <string>
#include <set>
#include <unordered_map>

namespace zap {

using inc_dir_set = std::set<std::string>;
using inc_dir_sets = std::unordered_map<std::string, inc_dir_set>;

}
