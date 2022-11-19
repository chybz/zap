#include <fstream>

#include <zap/cmake/toolchain_file.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

void
toolchain_file::write(
    const zap::toolchain& tc,
    const std::string& file
)
{
    std::ofstream ofs(file);

    write(ofs, "CMAKE_C_COMPILER", tc.cc_cmd());
    write(ofs, "CMAKE_CXX_COMPILER", tc.cxx_cmd());

    if (tc.has_compiler_launcher()) {
        write(ofs, "CMAKE_C_COMPILER_LAUNCHER", tc.compiler_launcher_cmd());
        write(ofs, "CMAKE_CXX_COMPILER_LAUNCHER", tc.compiler_launcher_cmd());
    }

    write(ofs, "CMAKE_FIND_ROOT_PATH", "${CMAKE_INSTALL_PREFIX}");
    write(ofs, "CMAKE_FIND_ROOT_PATH_MODE_LIBRARY", "ONLY");
    write(ofs, "CMAKE_FIND_ROOT_PATH_MODE_PACKAGE", "ONLY");
    write(ofs, "CMAKE_SKIP_BUILD_RPATH", "FALSE");
    write(ofs, "CMAKE_BUILD_WITH_INSTALL_RPATH", "FALSE");
    write(ofs, "CMAKE_INSTALL_RPATH", "${CMAKE_INSTALL_PREFIX}/lib");
    write(ofs, "CMAKE_INSTALL_RPATH_USE_LINK_PATH", "TRUE");
}

void
toolchain_file::write(
    std::ofstream& ofs,
    const std::string var,
    const std::string val
)
{
    bool quote = val.starts_with("${");

    ofs
        << "set(" << var << " "
        << (quote ? "\"" : "")
        << val
        << (quote ? "\"" : "")
        << ")\n"
        ;
}

}
