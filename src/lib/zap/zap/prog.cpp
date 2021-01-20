#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

args&
cat_args(args& to, const args& from)
{
    to.insert(to.end(), from.begin(), from.end());

    return to;
}

args
make_args(const std::string& cmd, const args& argv, const args& a)
{
    args all;

    all.reserve(argv.size() + a.size() + 1);
    all.push_back(cmd);
    cat_args(cat_args(all, argv), a);

    return all;
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
// prog_result
//
///////////////////////////////////////////////////////////////////////////////
bool
prog::empty() const
{ return cmd.empty(); }

std::string
prog::to_string(const args& a) const
{ return join(" ", make_args(cmd, argv, a)); }

void
prog::push_arg(const std::string& arg)
{ argv.push_back(arg); }

void
prog::push_args(const args& a)
{ cat_args(argv, a); }

void
prog::clear_args()
{ argv.clear(); }

prog_result
prog::run(const args& a, const run_opts& opts) const
{
    reproc::process p;
    reproc::options options;

    if (opts.redirect) {
        options.redirect.parent = true;
    } else {
        options.redirect.err.type = reproc::redirect::pipe;
    }

    auto complete_cmd = make_args(cmd, argv, a);
    auto complete_line = join(" ", complete_cmd);
    auto sec = p.start(complete_cmd, options);

    handle_error(complete_line, opts, sec);

    prog_result r;

    if (!opts.redirect) {
        drain(p, r.out, r.err);
    }

    auto [ status, ec ] = p.stop(options.stop);

    handle_error(complete_line, opts, ec, status);

    return r;
}

prog_result
prog::run(const args& a) const
{ return run(a, {}); }

prog_result
prog::run(const run_opts& opts) const
{ return run({}, opts); }

prog_result
prog::run_silent(const args& a) const
{ return run(a, { .redirect = false }); }

prog_result
prog::run_no_fail(const args& a) const
{ return run(a, { .mode = run_mode::no_fail }); }

prog_result
prog::run_silent_no_fail(const args& a) const
{ return run(a, { .mode = run_mode::no_fail, .redirect = false }); }

std::string
prog::get_line(const args& a) const
{
    auto res = run_silent_no_fail(a);

    return std::string{ res.get_line() };
}

void
prog::read_lines(prog_lines_cb cb, const args& a)
{
    reproc::process p;
    reproc::options options;

    options.redirect.err.type = reproc::redirect::pipe;

    run_opts opts{ run_mode::no_fail };
    auto complete_cmd = make_args(cmd, argv, a);
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
            "failed to start command: ", cmdline,
            ":\n", ec.message()
        );
    } else if (status) {
        die_unless(
            opts.mode == run_mode::no_fail,
            "command failed (", status, "): ", cmdline
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

prog_result
run(const std::string& cmd, const args& a)
{ return find_prog(cmd).run_silent_no_fail(a); }

std::string
get_line(const std::string& cmd, const args& a)
{ return find_prog(cmd).get_line(a); }

}
