#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

#include <zap/types.hpp>

namespace zap {

using args = std::vector<std::string>;

args&
cat_args(args& to, const args& from);

args
make_args(const std::string& cmd, const args& argv, const args& a);

struct prog_result
{
    std::string out;
    std::string err;

    string_views get_lines(const std::string& text) const;
    std::string_view get_line(const std::string& text) const;
    string_views get_lines() const;
    std::string_view get_line() const;
    string_views out_lines() const;
    std::string_view out_line() const;
    string_views err_lines() const;
    std::string_view err_line() const;
};

using prog_results = std::vector<prog_result>;

enum class run_mode
{
    std,
    no_fail
};

struct run_opts
{
    run_mode mode = run_mode::std;
    bool redirect = true;
    std::unordered_map<std::string, std::string> env;
};

using prog_lines_cb = std::function<void(string_views&)>;

struct prog
{
    std::string cmd;
    args argv;

    bool empty() const;

    std::string to_string(const args& a = {}) const;

    void push_arg(const std::string& arg);
    void push_args(const args& a);
    void clear_args();

    prog_result run(const args& a, const run_opts& opts) const;
    prog_result run(const args& a) const;
    prog_result run(const run_opts& opts = {}) const;
    prog_result run_silent(const args& a = {}) const;
    prog_result run_no_fail(const args& a = {}) const;
    prog_result run_silent_no_fail(const args& a = {}) const;

    std::string get_line(const args& a = {}) const;

    void read_lines(prog_lines_cb cb, const args& a = {});

private:
    void handle_error(
        const std::string& cmdline,
        const run_opts& opts,
        const std::error_code& ec,
        int status = 0
    ) const;
};

zap::prog
find_prog(const std::string& name);

prog_result
run(const std::string& cmd, const args& a = {});

std::string
get_line(const std::string& cmd, const args& a = {});

}
