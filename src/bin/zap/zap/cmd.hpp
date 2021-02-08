#pragma once

#include <variant>

#include <zap/command/empty.hpp>
#include <zap/command/configure.hpp>
#include <zap/command/build.hpp>
#include <zap/command/install.hpp>

namespace zap {

using cmd = std::variant<
    command::empty,
    command::configure,
    command::build,
    command::install
>;

} // namespace zap
