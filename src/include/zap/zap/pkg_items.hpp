#pragma once

#include <string>
#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

using pkg_items_map = strings_map;

void
merge_pkg_items(pkg_items_map& to, pkg_items_map& from);

}
