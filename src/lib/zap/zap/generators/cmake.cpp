#include <zap/generators/cmake.hpp>

#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::generators {

cmake::cmake(const toolchain& tc, const zap::project& p)
: generator(tc, p)
{}

cmake::~cmake()
{}

void
cmake::generate()
{
    generate_main();
    generate_targets(p().libs);
    generate_targets(p().bins);
    generate_targets(p().mods);
    generate_targets(p().tsts);
}

void
cmake::generate_main()
{
    open_list(p().root_dir);

    ofs_
        << "cmake_minimum_required(VERSION 3.12 FATAL_ERROR)\n"
        << "\nproject(" << p().name
        << " VERSION " << p().version
        << " LANGUAGES C CXX)\n"
        ;

    generate_includes();
    generate_pkg_configs();
    generate_cmake_components();
    generate_cmake_modules();


// "find_package(reproc++ REQUIRED)
// find_package(tomlplusplus REQUIRED)
// find_package(re2 REQUIRED)
// find_package(Taskflow REQUIRED)

// set(CMAKE_CXX_STANDARD 20)
// set(CMAKE_CXX_STANDARD_REQUIRED ON)
// set(CMAKE_CXX_EXTENSIONS OFF)
// set(CMAKE_POSITION_INDEPENDENT_CODE ON)

// add_subdirectory(lib/zap)
// add_subdirectory(bin/zap)
//         "
}

void
cmake::generate_includes()
{
    ofs_ << "\ninclude(GNUInstallDirs)\n";

    if (p().has_pkg_configs()) {
        ofs_ << "include(FindPkgConfig)\n";
    }
}

void
cmake::generate_pkg_configs()
{
    if (!p().has_pkg_configs()) {
        return;
    }

    ofs_ << "\n";

    for (const auto& pc : p().pkg_configs) {
        ofs_
            << "pkg_check_modules(" << pc
            << " REQUIRED IMPORTED_TARGET GLOBAL " << pc << ")\n"
            ;
    }
}

void
cmake::generate_cmake_components()
{
    ofs_ << "\n";

    for (const auto& p : p().cmake_components) {
        ofs_
            << "find_package(\n"
            << "    " << p.first << "\n"
            << zap::indent(
                "CONFIG",
                "REQUIRED",
                "COMPONENTS",
                zap::indent(p.second)
            )
            << ")\n"
            ;
    }
}

void
cmake::generate_targets(const zap::targets& ts)
{
    for (const auto& p : ts) {
        generate_target(p.second);
    }
}

void
cmake::generate_target(const zap::target& t)
{}

void
cmake::open_list(const std::string& dir)
{
    close_list();

    auto list = zap::cat_file(dir, "CMakeLists.txt");

    ofs_.open(list);

    zap::die_unless(
        ofs_.is_open(),
        "failed to open ", list
    );
}

void
cmake::close_list()
{
    if (ofs_.is_open()) {
        ofs_.close();
        ofs_.clear();
    }
}

}
