#include <string>
#include <unordered_map>

#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/toolchain_type.hpp>

namespace zap {

const std::string&
toolchain_type_name(toolchain_type tt)
{
    static std::unordered_map<toolchain_type, std::string> tnm = {
        { toolchain_type::unknown, "unknown" },
        { toolchain_type::gcc, "gcc" },
        { toolchain_type::clang, "clang" },
        { toolchain_type::apple_clang, "apple clang" },
        { toolchain_type::msvc, "msvc" }
    };

    auto it = tnm.find(tt);

    die_if(it == tnm.end(), "invalid toolchain type");

    return it->second;
}

const std::string&
toolchain_name(toolchain_type tt)
{
    static std::unordered_map<toolchain_type, std::string> tnm = {
        { toolchain_type::unknown, "unknown" },
        { toolchain_type::gcc, "gcc" },
        { toolchain_type::clang, "clang" },
        { toolchain_type::apple_clang, "clang" },
        { toolchain_type::msvc, "msvc" }
    };

    auto it = tnm.find(tt);

    die_if(it == tnm.end(), "invalid toolchain type");

    return it->second;
}

}
