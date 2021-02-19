#pragma once

#include <string>
#include <string_view>

#include <zap/types.hpp>
#include <zap/cmake/project.hpp>

namespace zap::cmake {

struct cmd
{
    zap::strings args;
    zap::string_set arg_keys;
};

class trace_parser
{
public:
    trace_parser();

    virtual ~trace_parser();

    project parse(
        const std::string& src_dir,
        const std::string& trace_file
    );

private:
    void parse_library(project& p, const std::string& line);
    void parse_library_includes(project& p, const std::string& line);

    bool ignore_library(const zap::string_set& arg_keys) const;

    cmd parse_cmd(const std::string& line) const;
    std::string make_cmd(const std::string& cmd) const;

    std::string project_source_dir_;
    std::string file_;
    std::string_view build_interface_;
};

}
