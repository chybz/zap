#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <utility>
#include <filesystem>

#include <zap/types.hpp>

namespace zap {

bool interactive();

void set_progress(std::size_t x, std::size_t n, std::size_t w = 50);
void clear_progress(std::size_t w = 50);

std::string home_directory();
std::string exe_path();
bool exists(const std::string_view& path);
bool directory_exists(const std::string_view& path);
bool file_exists(const std::string_view& path);
bool touch_file(const std::string& path);
bool truncate_file(const std::string& path, std::size_t size);
bool copy_file(const std::string& from, const std::string& to);
bool write_file(const std::string& file, const char* buffer, std::size_t size);
std::size_t file_size(const std::string_view& path);
std::size_t file_size_if_exists(const std::string_view& path);
std::size_t file_mtime(const std::string& path);
bool extension_is(const std::string_view& path, const std::string_view& ext);
bool file_extension_is(const std::string_view& path, const std::string_view& ext);
std::string slurp(const std::string& path);

bool mkpath(const std::string_view& path);
std::size_t rmpath(const std::string_view& path);
bool rmfile(const std::string_view& path);
bool mkfilepath(const std::string_view& path);
void rename(const std::string_view& from_path, const std::string_view& to_path);
std::string fullpath(const std::string_view& rel);
std::string fullpath(const std::string_view& dir, const std::string_view& file);
std::string basename(const std::string_view& path, const std::string_view& ext = {});
std::string dirname(const std::string_view& path);

strings find_files(const std::string_view& path, const std::string_view& re = {});
std::size_t file_count(const std::string_view& path, const std::string_view& re = {});
bool has_files(const std::string_view& path, const std::string_view& re = {});
strings find_dirs(const std::string_view& path);
std::size_t dir_count(const std::string_view& path);
std::pair<bool, std::string> unique_dir(const std::string_view& path);
std::pair<bool, std::string> unique_file(const std::string_view& path);
bool has_dirs(const std::string_view& path);

std::string empty_temp_dir(const std::string_view& base_dir);

std::size_t human_readable_size(const std::string& expr);

std::string human_readable_size(
    std::size_t size,
    const std::string& user_unit = "B"
);

std::string human_readable_file_size(const std::string& file);

std::string human_readable_rate(
    std::size_t count,
    std::size_t millisecs,
    const std::string& user_unit = "B"
);

std::string
plural(
    const std::string& base,
    const std::string& p,
    std::size_t count
);

bool has_spaces(const std::string& s);
bool has_spaces(const char* s);

void
split(
    const std::string_view& re,
    const std::string_view& expr,
    string_views& list
);

string_views split_lines(const std::string_view& text);

string_views map_lines(const std::string_view& re, const string_views& lines);

string_views
split_and_map_lines(const std::string_view& re, const std::string_view& text);

string_views
split_unique(const std::string_view& re, const std::string_view& text);

string_views
split_unique_and_map(
    const std::string_view& sre,
    const std::string_view& text,
    const std::string_view& mre
);

std::string
subst(const std::string& s, const std::string& re, const std::string& what);

void chomp(std::string& s);

strings glob(const std::string& expr);

bool match(const std::string& expr, const std::string& re);
bool imatch(const std::string& expr, const std::string& re);

bool
extract_delimited(
    std::string& from,
    std::string& to,
    unsigned char delim = ';',
    unsigned char quote = '\0'
);

std::string demangle_type_name(const std::string& mangled);

std::string escape(const char c);
std::string escape_for_string(const char c);
std::string escape(const std::string& s);

ssize_t send_file(int out_fd, int in_fd, off_t* offset, size_t count);

std::string
find_cmd(
    const std::string& cmd,
    const std::string& env_var = {}
);

std::string
try_find_cmd(
    const std::string& cmd,
    const std::string& env_var = {}
);

struct in_directory
{
    in_directory(const std::string& dir)
    : prev_dir(std::filesystem::current_path())
    { std::filesystem::current_path(dir); }

    ~in_directory()
    { std::filesystem::current_path(prev_dir); }

    std::string prev_dir;
};

template <typename Callable>
void
call_in_directory(const std::string& dir, Callable&& cb)
{
    in_directory d(dir);
    cb();
}

template <typename T>
inline
std::string
type_info(const T& t)
{ return demangle_type_name(typeid(t).name()); }

template <typename T>
inline
std::string
type_info()
{ return demangle_type_name(typeid(T).name()); }

inline
string_views
split(const std::string_view& re, const std::string_view& expr)
{
    string_views list;

    split(re, expr, list);

    return list;
}

template <typename T>
struct is_vector : public std::false_type{};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : public std::true_type{};


template <typename Container>
std::string
join_container(const std::string& sep, const Container& c)
{
    std::ostringstream oss;

    if (!c.empty()) {
        auto it = c.begin();

        oss << *it;

        while (++it != c.end()) {
            oss << sep << *it;
        }
    }

    return oss.str();
}

template <typename T, typename C>
std::string
join(const std::string& sep, const C<T>& c)
{
    if constexpr (std::is_same_v<C<T>, >)
}


template <typename Container>
std::string
join(const std::string& sep, const Container& c)


template <typename Tuple, std::size_t... Is>
void
join_tuple_impl(
    std::ostringstream& oss,
    const std::string& sep,
    const Tuple& t,
    std::index_sequence<Is...>
)
{
    // (from http://stackoverflow.com/a/6245777/273767)
    using swallow = int[]; // guarantees left to right order

    (void) swallow{
        0,
        (
            void(
                oss << (Is == 0 ? "" : sep) << std::get<Is>(t)
            ),
            0
        )...
    };
}

template <typename... Args>
void
join_tuple(
    std::ostringstream& oss,
    const std::string& sep,
    const std::tuple<Args...>& t
)
{ join_tuple_impl(oss, sep, t, std::index_sequence_for<Args...>{}); }

template <
    typename... Args,
    typename Enable = std::enable_if_t<(sizeof...(Args) > 1)>
>
std::string
join(const std::string& sep, Args&&... args)
{
    std::ostringstream oss;

    join_tuple(oss, sep, std::make_tuple(std::forward<Args>(args)...));

    return oss.str();
}

inline
std::string
indent(const std::string& what, const std::string& pad = "    ")
{
    std::string sep = "\n";
    std::string s;

    if (what.size() > 0) {
        s = pad + join(sep + pad, split(sep, what));
    }

    return s;
}

template <typename T>
class reverse
{
public:
  explicit reverse(T& iterable)
  : iterable_{iterable}
  {}

  auto begin() const
  { return std::rbegin(iterable_); }

  auto end() const
  { return std::rend(iterable_); }

private:
  T& iterable_;
};

template <typename... Args>
std::string
cat(Args&&... args)
{ return join("", std::forward<Args>(args)...); }

template <typename... Args>
std::string
cat_dir(Args&&... args)
{ return join("/", std::forward<Args>(args)...); }

template <typename... Args>
std::string
cat_file(Args&&... args)
{ return cat_dir(std::forward<Args>(args)...); }

template <typename T>
inline
T to_num(const std::string& s)
{ return ((T) std::strtoull(s.c_str(), NULL, 10)); }

}
