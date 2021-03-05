#pragma once

#include <string>
#include <string_view>

#include <zap/toolchain.hpp>
#include <zap/types.hpp>
#include <zap/cmake/project.hpp>

namespace zap::cmake {

struct cmd
{
    zap::strings args;
    zap::string_set arg_keys;

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

    const project& static_project() const;
    const project& shared_project() const;

private:
    void parse_library(const std::string& line);

    void parse_library_includes(
        const std::string& src_dir,
        const std::string& line
    );

    bool ignore_library(const cmd& c) const;

    cmd parse_cmd(const std::string& line) const;
    std::string make_cmd(const std::string& cmd) const;

    void add_alias(const std::string& from, const std::string& to);

    void add_alias(
        project& p,
        const std::string& from,
        const std::string& to
    );

    void set_library_interface(
        const std::string& name,
        const std::string& dir
    );

    void set_library_interface(
        project& p,
        const std::string& name,
        const std::string& dir
    );

    void set_project_dirs(const std::string& dir);

    const zap::toolchain& tc_;
    std::string project_source_dir_;
    std::string file_;
    std::string_view build_interface_;
    project static_;
    project shared_;
};

}
