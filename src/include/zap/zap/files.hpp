#pragma once

#include <string>

#include <zap/vector_map.hpp>

namespace zap {

using files = vector_map<std::string>;

void
add_files(files& f, const std::string& dir, const std::string& re);

}
