#include <zap/generators/cmake.hpp>

#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::generators {

cmake::cmake(const zap::env& e, const zap::project& p)
: generator(e, p)
{}

cmake::~cmake()
{ close_list(); }

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
        << " LANGUAGES"
        ;

    if (p().langs.c) {
        ofs_ << " C";
    }

    if (p().langs.cpp) {
        ofs_ << " CXX";
    }

    ofs_
        << ")\n"
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
{}

void
cmake::generate_pkg_configs()
{}

void
cmake::generate_cmake_components()
{}

void
cmake::generate_cmake_modules()
{}

void
cmake::generate_targets(const zap::targets& ts)
{}

void
cmake::generate_target(const zap::target& t)
{}

void
cmake::generate_exe_target(const zap::target& t)
{}

void
cmake::generate_lib_target(const zap::target& t)
{}

void
cmake::generate_target_sources(const zap::target& t)
{}

void
cmake::open_list(const std::string& dir)
{}

void
cmake::close_list()
{}

}
