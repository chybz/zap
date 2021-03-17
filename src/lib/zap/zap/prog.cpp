#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

strings&
cat_args(strings& to, const strings& from)
{
    to.insert(to.end(), from.begin(), from.end());

    return to;
}

strings
cat_args(const strings& a, const strings& b)
{
    strings all;

    all.reserve(a.size() + b.size());
    all.insert(all.end(), a.begin(), a.end());
    all.insert(all.end(), b.begin(), b.end());

    return all;
}

strings
make_args(const std::string& cmd, const strings& args)
{
    strings all;

    all.reserve(args.size() + 1);
    all.push_back(cmd);
    cat_args(all, args);

    return all;
}

string_map&
merge_env(string_map& to, const string_map& from)
{
    for (const auto& p : from) {
        to.try_emplace(p.first, p.second);
    }

    return to;
}

string_map
merge_env(const string_map& a, const string_map& b)
{
    string_map m;

    merge_env(m, a);
    merge_env(m, b);

    return m;
}

void
drain(reproc::process& p, std::string& out, std::string& err)
{
    auto ec = reproc::drain(
        p,
        reproc::sink::string(out),
        reproc::sink::string(err)
    );

    if (ec) {
        die("error draining: ", ec.message());
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// prog result drain by line
//
///////////////////////////////////////////////////////////////////////////////
class prog_line_drain
{
public:
    explicit prog_line_drain(
        prog_lines_cb& cb,
        std::string& r
    ) noexcept
    : cb_(cb),
    r_(r)
    {}

    std::error_code operator()(
        reproc::stream stream,
        const uint8_t* buffer,
        std::size_t size
    )
    {
        r_.append(reinterpret_cast<const char*>(buffer), size);

        std::string_view data = r_;

        auto pos = data.rfind('\n');
        std::size_t sep_count = 1;

        if (pos != std::string_view::npos) {
            if (pos > 0 && data[pos - 1] == '\r')  {
                --pos;
                ++sep_count;
            }

            auto lines = split_lines(data.substr(0, pos));

            cb_(lines);

            r_.erase(0, pos + sep_count);
        }

        return {};
    }

private:
    prog_lines_cb& cb_;
    std::string& r_;
};

///////////////////////////////////////////////////////////////////////////////
//
// prog_result
//
///////////////////////////////////////////////////////////////////////////////
string_views
prog_result::get_lines(const std::string& text) const
{ return split_lines(text); }

std::string_view
prog_result::get_line(const std::string& text) const
{
    auto lines = get_lines(text);

    return lines.empty() ? std::string_view{} : lines.front();
}

string_views
prog_result::get_lines() const
{ return get_lines(out); }

std::string_view
prog_result::get_line() const
{ return get_line(out); }

string_views
prog_result::out_lines() const
{ return get_lines(out); }

std::string_view
prog_result::out_line() const
{ return get_line(out); }

string_views
prog_result::err_lines() const
{ return get_lines(err); }

std::string_view
prog_result::err_line() const
{ return get_line(err); }

///////////////////////////////////////////////////////////////////////////////
//
// prog
//
///////////////////////////////////////////////////////////////////////////////
bool
prog::empty() const
{ return cmd.empty(); }

std::string
prog::to_string(const strings& a) const
{ return join(" ", make_args(cmd, cat_args(args, a))); }

void
prog::push_arg(const std::string& arg)
{ args.push_back(arg); }

void
prog::push_args(const strings& a)
{ cat_args(args, a); }

void
prog::clear_args()
{ args.clear(); }

prog_result
prog::run(const prog_opts& po) const
{
    reproc::process p;
    reproc::options options{ { .extra = merge_env(env, po.env) } };

    if (po.opts.redirect) {
        options.redirect.parent = true;
    } else {
        options.redirect.err.type = reproc::redirect::pipe;
    }

    auto complete_cmd = make_args(cmd, cat_args(args, po.args));
    auto complete_line = join(" ", complete_cmd);
    auto sec = p.start(complete_cmd, options);

    handle_error(complete_line, po.opts, sec);

    prog_result r;

    if (!po.opts.redirect) {
        drain(p, r.out, r.err);
    }

    auto [ status, ec ] = p.stop(options.stop);

    handle_error(complete_line, po.opts, ec, status);

    return r;
}

prog_result
prog::run_silent(const prog_opts& po) const
{
    prog_opts tpo{ po };

    tpo.opts.redirect = false;

    return run(tpo);
}

prog_result
prog::run_no_fail(const prog_opts& po) const
{
    prog_opts tpo{ po };

    tpo.opts.mode = run_mode::no_fail;

    return run(tpo);
}

prog_result
prog::run_silent_no_fail(const prog_opts& po) const
{
    prog_opts tpo{ po };

    tpo.opts.redirect = false;
    tpo.opts.mode = run_mode::no_fail;

    return run(tpo);
}

std::string
prog::get_line(const prog_opts& po) const
{
    auto res = run_silent_no_fail(po);

    return std::string{ res.get_line() };
}

void
prog::read_lines(prog_lines_cb cb, const prog_opts& po)
{
    reproc::process p;
    reproc::options options{ { .extra = merge_env(env, po.env) } };

    options.redirect.err.type = reproc::redirect::pipe;

    run_opts opts{ run_mode::no_fail };
    auto complete_cmd = make_args(cmd, cat_args(args, po.args));
    auto complete_line = join(" ", complete_cmd);
    auto sec = p.start(complete_cmd, options);

    handle_error(complete_line, opts, sec);

    prog_result r;

    auto dec = reproc::drain(
        p,
        prog_line_drain(cb, r.out),
        prog_line_drain(cb, r.err)
    );

    if (dec) {
        die("error draining: ", dec.message());
    }

    auto [ status, ec ] = p.stop(options.stop);

    handle_error(complete_line, opts, ec, status);
}

void
prog::handle_error(
    const std::string& cmdline,
    const run_opts& opts,
    const std::error_code& ec,
    int status
) const
{
    if (ec == std::errc::no_such_file_or_directory) {
        die("command not found: ", cmd);
    } else if (ec) {
        die_unless(
            opts.mode == run_mode::no_fail,
            "command failed: ", cmdline,
            ":\n", ec.message()
        );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
zap::prog
find_prog(const std::string& name)
{ return { find_cmd(name) }; }

void
try_find_prog(const std::string& name, prog& p)
{ p.cmd = try_find_cmd(name); }

prog_result
run(const std::string& cmd, const prog_opts& po)
{ return find_prog(cmd).run_silent_no_fail(po); }

std::string
get_line(const std::string& cmd, const prog_opts& po)
{ return find_prog(cmd).get_line(po); }

}
