#pragma once

#include <fstream>

#include <zap/generator.hpp>

namespace zap::generators {

class cmake : public zap::generator
{
public:
    cmake(const zap::env& e, const zap::project& p);
    virtual ~cmake();

    void generate() final;

private:
    void generate_main();
    void generate_includes();
    void generate_pkg_configs();
    void generate_cmake_components();
    void generate_cmake_modules();
    void generate_targets(const zap::targets& ts);
    void generate_target(const zap::target& t);
    void generate_exe_target(const zap::target& t);
    void generate_lib_target(const zap::target& t);
    void generate_target_sources(const zap::target& t);

    void open_list(const std::string& dir);
    void close_list();

    std::ofstream ofs_;
};

}
