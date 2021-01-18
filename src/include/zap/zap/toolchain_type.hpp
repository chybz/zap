#pragma once

#include <string>

namespace zap {

enum toolchain_type
{
    unknown,
    gcc,
    clang,
    apple_clang,
    msvc
};

const std::string&
toolchain_type_name(toolchain_type tt);

const std::string&
toolchain_name(toolchain_type tt);

inline
std::ostream&
operator<<(std::ostream& os, toolchain_type tt)
{
    os << toolchain_type_name(tt);

    return os;
}

}
