#pragma once

#include <string>
#include <string_view>

#include <re2/re2.h>

#include <zap/toolchain.hpp>
#include <zap/types.hpp>
#include <zap/cmake/project.hpp>

namespace zap::cmake {

struct cmd
{
    std::string subject; // The first arg, for convenience
    zap::strings args;
    zap::string_set arg_keys;
    std::string file;
    std::size_t frame;

    bool has(const std::string& key) const;
};

class trace_parser
{
public:
    trace_parser(const zap::toolchain& tc);

    virtual ~trace_parser();

    void parse(
        const std::string& src_dir,
        const std::string& trace_file
    );

    void post_install(const std::string& inst_dir);

    const zap::cmake::project& project() const;

private:
    void handle_deps();

    void parse_subdirectory(const std::string& line);

    void parse_library(const std::string& line);

    void parse_library_sources(const std::string& line);

    void parse_library_sources(
        const std::string& lib,
        const zap::strings& args
    );

    void parse_library_includes(const std::string& line);

    void parse_library_deps(const std::string& line);

    bool not_a_library(const std::string& s) const;

    zap::string_views parse_build_interface(const std::string& s) const;
    zap::string_views parse_install_interface(const std::string& s) const;

    zap::string_views parse_interface(
        const std::string& s,
        const std::string_view& interface
    ) const;

    bool ignore_library(const cmd& c) const;

    cmd parse_cmd(const std::string& line) const;
    std::string make_cmd(const std::string& cmd) const;

    const zap::toolchain& tc_;
    re2::RE2 hdr_re_;
    std::string src_dir_;
    std::string inst_dir_;
    std::string file_;
    std::string_view build_interface_;
    std::string_view install_interface_;
    std::string subdir_;
    zap::cmake::project p_;
    zap::string_set seen_libs_;
    zap::string_set_map deps_;
    zap::string_set_map rev_deps_;
};

}
