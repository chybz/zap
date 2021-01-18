#pragma once

#include <string>

#include <zap/list_map.hpp>

namespace zap {

using files = list_map<std::string>;

void
add_files(files& f, const std::string& dir, const std::string& re);

}
