#pragma once

#include <variant>

#include <zap/command/empty.hpp>
#include <zap/command/scan.hpp>
#include <zap/command/build.hpp>
#include <zap/command/install.hpp>

namespace zap {

using cmd = std::variant<
    command::empty,
    command::scan,
    command::build,
    command::install
>;

} // namespace zap
