#pragma once

#include <string>
#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

using pkg_items_map = std::unordered_map<std::string, zap::strings>;

void
merge_items(pkg_items_map& to, pkg_items_map& from);

}
