#pragma once

#include <variant>

#include <zap/commands/empty.hpp>
#include <zap/commands/configure.hpp>
#include <zap/commands/build.hpp>
#include <zap/commands/install.hpp>

namespace zap {

using cmd = std::variant<
    commands::empty,
    commands::configure,
    commands::build,
    commands::install
>;

} // namespace zap
