#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

#include <zap/types.hpp>

namespace zap {

strings&
cat_args(strings& to, const strings& from);

strings
cat_args(const strings& a, const strings& b);

strings
make_args(const std::string& cmd, const strings& args);

string_map&
merge_env(string_map& to, const string_map& from);

string_map
merge_env(const string_map& a, const string_map& b);

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
    string_map env;
};

struct prog_opts
{
    strings args;
    string_map env;
    run_opts opts;
};

using prog_lines_cb = std::function<void(string_views&)>;

struct prog
{
    std::string cmd;
    strings args;
    string_map env;

    bool empty() const;

    std::string to_string(const strings& a = {}) const;

    void push_arg(const std::string& arg);
    void push_args(const strings& a);
    void clear_args();

    prog_result run(const prog_opts& po = {}) const;

    prog_result run_silent(const prog_opts& po = {}) const;
    prog_result run_no_fail(const prog_opts& po = {}) const;
    prog_result run_silent_no_fail(const prog_opts& po = {}) const;

    std::string get_line(const prog_opts& po = {}) const;

    void read_lines(prog_lines_cb cb, const prog_opts& po = {});

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

void
try_find_prog(const std::string& name, prog& p);

prog_result
run(const std::string& cmd, const prog_opts& po = {});

std::string
get_line(const std::string& cmd, const prog_opts& po = {});

}
