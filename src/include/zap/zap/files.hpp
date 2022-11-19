#pragma once

#include <string>

#include <zap/types.hpp>

namespace zap {

using files = string_set;

void
add_files(files& f, const std::string& dir, const std::string& re);

}
