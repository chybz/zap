#ifdef __unix__
#include <unistd.h>
#include <cxxabi.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include <limits.h>
#endif

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#include <ios>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>

#include <re2/re2.h>

#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

const std::size_t progress_ratio_width = 3;
const std::size_t progress_bar_chars = 4;
const std::size_t progress_bar_extra_width =
    progress_ratio_width + progress_bar_chars;
const std::size_t kilobytes = 1024;
const std::size_t megabytes = kilobytes * 1024;
const std::size_t gigabytes = megabytes * 1024;
const std::size_t terabytes = gigabytes * 1024;
const std::size_t petabytes = terabytes * 1024;

struct human_size_conv {
    const char* unit;
    std::size_t scale;
};

bool
interactive()
{
    return (
        ::isatty(::fileno(::stdin)) == 1
        &&
        ::isatty(::fileno(::stdout)) == 1
        &&
        (std::getenv("HARNESS_ACTIVE") == nullptr)
    );
}

void set_progress(std::size_t x, std::size_t n, std::size_t w)
{
    if (!interactive()) {
        return;
    }

    float ratio = x / (float) n;
    std::size_t c = ratio * w;

    std::cout << "\r" << std::flush;

    std::cout << std::setw(3) << (std::size_t) (ratio * 100) << "% [";

    for (std::size_t x = 0; x < c; x++) {
        std::cout << "=";
    }

    for (std::size_t x = c; x < w; x++) {
        std::cout << " ";
    }

    std::cout << "]" << std::flush;
}

void clear_progress(std::size_t w)
{
    if (!interactive()) {
        return;
    }

    for (std::size_t x = 0; x < w + progress_bar_extra_width; x++) {
        std::cout << " ";
    }

    std::cout << "\r" << std::flush;
}

std::string
home_directory()
{
    const char* home = nullptr;
    std::string home_dir;

    home = getenv("HOME");

    if (home != nullptr) {
        home_dir = home;
    }

    if (home_dir.size() == 0) {
        throw std::runtime_error("failed to find home directory");
    }

    return home_dir;
}

std::string
exe_path()
{
    char buf[PATH_MAX];

    ::memset(buf, 0, sizeof(buf));

#if defined(__linux__)
        ssize_t r = readlink("/proc/self/exe", buf, sizeof(buf));
#elif defined(__APPLE__)
        char tbuf[PATH_MAX];
        uint32_t bufsize = sizeof(tbuf);
        int r = _NSGetExecutablePath(tbuf, &bufsize);
        realpath(tbuf, buf);
#else
        throw std::runtime_error("exe_path not (yet) implemented");
        // TODO: FreeBSD readlink("/proc/curproc/file", buf, bufsize);
        // TODO: Solaris readlink("/proc/self/path/a.out", buf, bufsize);
#endif

        if (r < 0) {
            throw std::runtime_error("failed to find program path");
        }

        std::string path = buf;

        return path;
}

template <typename Path>
std::filesystem::path
normalized(const Path& p)
{
    namespace fs = std::filesystem;

    return fs::weakly_canonical(fs::path(p));
}

bool
exists(const std::string_view& path)
{
    namespace fs = std::filesystem;
    fs::path ppath(path);

    return fs::exists(ppath);
}

bool
directory_exists(const std::string_view& path)
{
    namespace fs = std::filesystem;
    fs::path ppath(path);

    return (
        fs::exists(ppath)
        &&
        fs::is_directory(ppath)
    );
}

bool
file_exists(const std::string_view& path)
{
    namespace fs = std::filesystem;
    fs::path ppath(path);

    return (
        fs::exists(ppath)
        &&
        fs::is_regular_file(ppath)
    );
}

bool
touch_file(const std::string& path)
{
    if (!mkfilepath(path)) {
        return false;
    }

    std::filebuf fbuf;

    fbuf.open(
        path,
        std::ios_base::in
        | std::ios_base::out
        | std::ios_base::app
        | std::ios_base::binary
    );

    if (!fbuf.is_open()) {
        return false;
    }

    fbuf.close();

    return true;
}

bool
truncate_file(const std::string& path, std::size_t size)
{
    if (!touch_file(path)) {
        return false;
    }

    if (truncate(path.c_str(), size) != 0) {
        return false;
    }

    return true;
}

bool
copy_file(const std::string& from, const std::string& to)
{
    namespace fs = std::filesystem;

    if (!file_exists(from)) {
        return false;
    }

    std::string to_;

    if (directory_exists(to)) {
        // New file in existing directory
        to_ = fullpath(to, basename(from));
    } else {
        to_ = to;
    }

#if defined(__linux__)
    int src = ::open(from.c_str(), O_RDONLY, 0);

    sysdie_if(src == -1, "failed to open file: ", from);

    int dst = ::creat(to_.c_str(), 0644);

    if (dst == -1) {
        ::close(src);
        die("failed to create file: ", to_);
    }

    auto ret = send_file(dst, src, nullptr, file_size(from));

    ::close(src);
    ::close(dst);

    sysdie_if(ret == -1, "failed to send file: ", from);
#elif defined(__APPLE__)
    // Sadly, sendfile doesn't support destination being a file
    std::ifstream source(from, std::ios::binary);
    std::ofstream dest(to_, std::ios::binary | std::ios::trunc);

    if (!(source.is_open() && dest.is_open())) {
        return false;
    }

    dest << source.rdbuf();

    source.close();
    dest.close();
#endif

    return true;
}

bool
write_file(const std::string& file, const char* buffer, std::size_t size)
{
    if (!mkfilepath(file)) {
        return false;
    }

    std::ofstream ofs(file, std::ios::binary | std::ios::trunc);

    if (!ofs.is_open()) {
        return false;
    }

    ofs.write(buffer, size);
    ofs.close();

    return true;
}

std::size_t
file_size(const std::string_view& path)
{
    namespace fs = std::filesystem;
    fs::path ppath(path);

    return fs::file_size(ppath);
}

std::size_t
file_size_if_exists(const std::string_view& path)
{
    return
        file_exists(path)
        ? file_size(path)
        : 0
        ;
}

std::size_t
file_mtime(const std::string& path)
{
    struct stat statbuf;

    if (::stat(path.c_str(), &statbuf) != 0) {
        return 0;
    }

    return statbuf.st_mtim.tv_sec;
}

bool
extension_is(const std::string_view& path, const std::string_view& ext)
{
    namespace fs = std::filesystem;

    return fs::path(path).extension() == ext;
}

bool
file_extension_is(const std::string_view& path, const std::string_view& ext)
{
    return
        file_exists(path)
        &&
        extension_is(path, ext)
        ;
}

std::string
slurp(const std::string& path)
{
    std::string bytes;

    if (file_exists(path)) {
        std::ifstream ifs(path, std::ios::binary);

        if (ifs.is_open()) {
            auto fsize = file_size(path);

            bytes.resize(fsize);

            ifs.read(bytes.data(), fsize);
            ifs.close();
        }
    }

    return bytes;
}

bool
mkpath(const std::string_view& path)
{
    namespace fs = std::filesystem;
    fs::path ppath(path);

    if (!fs::exists(ppath)) {
        return fs::create_directories(ppath);
    }

    return true;
}

std::size_t
rmpath(const std::string_view& path)
{
    if (!directory_exists(path)) {
        return 0;
    }

    namespace fs = std::filesystem;
    fs::path ppath(path);

    return fs::remove_all(ppath);
}

bool
rmfile(const std::string_view& path)
{
    if (!file_exists(path)) {
        return false;
    }

    namespace fs = std::filesystem;
    fs::path ppath(path);

    return fs::remove(ppath);
}

bool
mkfilepath(const std::string_view& path)
{
    namespace fs = std::filesystem;

    bool ret = true;
    auto ppath = normalized(path);

    die_unless(
        ppath.has_filename(),
        "path has no filename"
    );

    fs::path parent = ppath.parent_path();
    fs::path filename = ppath.filename();

    if (!fs::exists(parent)) {
        ret = fs::create_directories(parent);
    }

    return ret;
}

void
rename(const std::string_view& from_path, const std::string_view& to_path)
{
    namespace fs = std::filesystem;
    fs::path from_ppath(from_path);
    fs::path to_ppath(to_path);

    fs::rename(from_ppath, to_ppath);
}

std::string
fullpath(const std::string_view& rel)
{ return normalized(rel).string(); }

std::string
fullpath(const std::string_view& dir, const std::string_view& file)
{
    namespace fs = std::filesystem;

    auto path = normalized(dir);

    path /= file;

    return normalized(path).string();
}

std::string
basename(const std::string_view& path, const std::string_view& ext)
{
    namespace fs = std::filesystem;

    auto ppath = normalized(path);

    if (ext.empty()) {
        return ppath.filename().string();
    }

    auto bn = ppath.filename().string();
    auto pos = bn.rfind(ext);

    if (
        pos != std::string::npos
        &&
        pos + ext.size() == bn.size()
    ) {
        bn.erase(pos);
    }

    return bn;
}

std::string
dirname(const std::string_view& path)
{
    namespace fs = std::filesystem;

    auto ppath = normalized(path);

    return ppath.parent_path().string();
}

template <typename Callable, typename... Args>
void
find_files_core(
    const std::string_view& path,
    const std::string_view& re,
    Callable&& cb,
    Args&&... args
)
{
    namespace fs = std::filesystem;

    if (!directory_exists(path)) {
        return;
    }

    std::string pattern{ re2::RE2::QuoteMeta(path) };

    if (pattern.back() != '/') {
        pattern += '/';
    }

    if (re.empty()) {
        pattern += "(.*)";
    } else {
        pattern += '(';
        pattern.append(re.data(), re.size());
        pattern += ')';
    }

    re2::RE2 fre(pattern);

    for (const auto& p : fs::recursive_directory_iterator(path)) {
        if (!p.is_regular_file()) {
            continue;
        }

        auto match = re2::RE2::FullMatch(
            p.path().string(),
            fre,
            std::forward<Args>(args)...
        );

        if (match) {
            cb();
        }
    }
}

strings
find_files(const std::string_view& path, const std::string_view& re)
{
    strings files;

    std::string file;

    find_files_core(
        path,
        re,
        [&] { files.push_back(file); },
        &file
    );

    return files;
}

std::size_t
file_count(const std::string_view& path, const std::string_view& re)
{
    std::size_t count = 0;

    find_files_core(
        path,
        re,
        [&] { ++count; }
    );

    return count;
}

bool
has_files(const std::string_view& path, const std::string_view& re)
{ return file_count(path, re) > 0; }

strings
find_dirs(const std::string_view& path)
{
    namespace fs = std::filesystem;

    strings dirs;

    if (directory_exists(path)) {
        for (const auto& p : fs::directory_iterator(path)) {
            if (p.is_directory()) {
                dirs.push_back(p.path().filename().string());
            }
        }
    }

    return dirs;
}

std::size_t
dir_count(const std::string_view& path)
{
    namespace fs = std::filesystem;

    std::size_t count = 0;

    if (directory_exists(path)) {
        for (const auto& p : fs::directory_iterator(path)) {
            count += p.is_directory();
        }
    }

    return count;
}

std::pair<bool, std::string>
unique_dir(const std::string_view& path)
{
    namespace fs = std::filesystem;

    std::size_t count = 0;
    fs::path last;

    if (directory_exists(path)) {
        for (const auto& p : fs::directory_iterator(path)) {
            count += p.is_directory();
            last = p;
        }
    }

    if (count == 1) {
        return { true, last.filename().string() };
    }

    return { false, {} };
}

std::pair<bool, std::string>
unique_file(const std::string_view& path)
{
    namespace fs = std::filesystem;

    std::size_t count = 0;
    fs::path last;

    if (directory_exists(path)) {
        for (const auto& p : fs::directory_iterator(path)) {
            count += p.is_regular_file();
            last = p;
        }
    }

    if (count == 1) {
        return { true, last.filename().string() };
    }

    return { false, {} };
}

bool
has_dirs(const std::string_view& path)
{ return dir_count(path) > 0; }

bool
dir_is_empty(const std::string_view& path)
{
    if (!directory_exists(path)) {
        return true;
    }

    return !has_dirs(path) && !has_files(path);
}

std::string
empty_temp_dir(const std::string_view& base_dir)
{
    namespace fs = std::filesystem;

    fs::path tmp_dir(base_dir);
    std::size_t retries = 10;
    std::random_device dev;
    std::mt19937 prng(dev());
    std::uniform_int_distribution<std::size_t> rand(0);
    fs::path path;

    while (true) {
        std::stringstream ss;
        ss << std::hex << rand(prng);
        path = tmp_dir / ss.str();

        // true if the directory was created.
        if (fs::create_directories(path)) {
            break;
        }

        if (--retries == 0) {
            die("could not create temporary directory");
        }
    }

    return path.string();
}

std::size_t
human_readable_size(const std::string& expr)
{
    static std::vector<human_size_conv> ct = {
        { "K", kilobytes },
        { "M", megabytes },
        { "G", gigabytes },
        { "T", terabytes },
        { "P", petabytes }
    };

    re2::RE2 sizeexpr("(\\d+)(?:(K|M|G|T|P))?");
    std::size_t size;
    std::string unit;
    std::size_t scale = 1;

    if (re2::RE2::FullMatch(expr, sizeexpr, &size, &unit)) {
        for (const auto& c : ct) {
            if (unit == c.unit) {
                scale = c.scale;
                break;
            }
        }

        size *= scale;
    } else {
        die("invalid size: ", expr);
    }

    return size;
}

std::string
human_readable_size(std::size_t size, const std::string& user_unit)
{
    static std::vector<human_size_conv> ct = {
        { "P", petabytes },
        { "T", terabytes },
        { "G", gigabytes },
        { "M", megabytes },
        { "K", kilobytes }
    };

    std::ostringstream oss;
    double value = size;
    const char* unit = "";
    std::size_t scale = 1;

    for (const auto& c : ct) {
        if (size >= c.scale) {
            scale = c.scale;
            unit = c.unit;
            break;
        }
    }

    value /= scale;

    char buf[50 + 1];

    std::snprintf(buf, sizeof(buf), "%.2f", value);

    oss << buf << unit << user_unit;

    return oss.str();
}

std::string
human_readable_file_size(const std::string& file)
{ return human_readable_size(file_size(file)); }

std::string
human_readable_rate(
    std::size_t count,
    std::size_t millisecs,
    const std::string& user_unit
)
{
    double secs = millisecs / 1000.0;

    double count_per_sec = count;
    count_per_sec /= secs;

    std::ostringstream oss;

    oss
        << human_readable_size(
            std::llround(count_per_sec),
            user_unit
        )
        << "/s"
        ;

    return oss.str();
}

std::string
plural(
    const std::string& base,
    const std::string& p,
    std::size_t count
)
{
    return
        count > 1
        ? base + p
        : base
        ;
}

std::string
toupper(const std::string& s)
{
    auto upper(s);

    std::transform(
        upper.begin(),
        upper.end(),
        upper.begin(),
        ::toupper
    );

    return upper;
}

bool
has_spaces(const std::string& s)
{
    bool has_spaces = std::any_of(
        s.begin(), s.end(),
        [](auto c) { return std::isspace(c); }
    );

    return has_spaces;
}

bool
has_spaces(const char* s)
{ return has_spaces(std::string(s)); }

void
split(
    const std::string_view& re,
    const std::string_view& expr,
    string_views& list
)
{
    list.clear();

    if (expr.empty()) {
        return;
    }

    std::string cap_re = "(";
    cap_re.append(re.begin(), re.end());
    cap_re += ")";

    re2::RE2 pattern(cap_re);
    re2::StringPiece input(expr);
    re2::StringPiece delim;

    auto begin = expr.begin();

    while (re2::RE2::FindAndConsume(&input, pattern, &delim)) {
        list.push_back({ begin, delim.begin() });
        begin = delim.end();
    }

    if (!list.empty()) {
        list.push_back({ delim.end(), expr.end() });
    } else {
        list.push_back(expr);
    }
}

string_views
split_lines(const std::string_view& text)
{
    string_views lines;

    split("\n", text, lines);

    return lines;
}

string_views
map_lines(const std::string_view& re, const string_views& lines)
{
    string_views result;

    re2::RE2 pattern(re);
    re2::StringPiece match;

    for (const auto& l : lines) {
        if (re2::RE2::FullMatch(l, pattern, &match)) {
            result.push_back({ match.begin(), match.end() });
        }
    }

    return result;
}

string_views
split_and_map_lines(const std::string_view& re, const std::string_view& text)
{ return map_lines(re, split_lines(text)); }

string_views
split_unique(const std::string_view& re, const std::string_view& text)
{
    string_view_set seen;
    string_views result;

    for (const auto& item : split(re, text)) {
        if (seen.count(item) == 0) {
            seen.insert(item);
            result.emplace_back(item);
        }
    }

    return result;
}

string_views
split_unique_and_map(
    const std::string_view& sre,
    const std::string_view& text,
    const std::string_view& mre
)
{ return map_lines(mre, split_unique(sre, text)); }

bool
match(const std::string& expr, const std::string& re)
{
    std::regex mre(re);

    return std::regex_match(expr, mre);
}

bool
match(const std::string& expr, const std::string& re, std::smatch& m)
{
    std::regex mre(re);

    return std::regex_match(expr, m, mre);
}

bool
imatch(const std::string& expr, const std::string& re)
{
    std::regex mre(
        re,
        std::regex_constants::ECMAScript
        | std::regex_constants::icase
    );

    return std::regex_match(expr, mre);
}

std::string
subst(const std::string& s, const std::string& re, const std::string& what)
{
    return std::regex_replace(
        s,
        std::regex(re),
        what
    );
}

void
chomp(std::string& s)
{
    std::string::size_type pos = s.find_last_not_of("\r\n");

    if (pos != std::string::npos) {
        s = s.substr(0, pos + 1);
    } else if (s.size() > 0) {
        s.clear();
    }
}

strings
glob(const std::string& expr)
{
    glob_t g;
    auto patterns = split("\\s+", expr);
    strings matches;
    int flags = 0;

    std::string spat;

    spat.reserve(1024); // Yeah...

    if (!patterns.empty()) {
        for (const auto& pattern : patterns) {
            spat.assign(pattern.begin(), pattern.end());
            glob(spat.c_str(), flags, NULL, &g);

            // Calling with GLOB_APPEND the first time segfaults...
            // (glob() surely tries to free unallocated data)
            flags |= GLOB_APPEND;
        }

        if (g.gl_pathc > 0) {
            for (size_t i = 0; i < g.gl_pathc; i++) {
                matches.push_back(g.gl_pathv[i]);
            }
        }

        globfree(&g);
    }

    return matches;
}

bool
extract_delimited(
    std::string& from,
    std::string& to,
    unsigned char delim,
    unsigned char quote
)
{
    to.clear();

    // Find the first semicolon not surrounded by the quote char
    // (if specified)
    unsigned int quote_count = 0;
    bool found = false;
    std::string::iterator iter;

    for (iter = from.begin(); iter != from.end(); ++iter) {
        if (quote != '\0' && *iter == quote) {
            quote_count++;
        } else if (*iter == delim && (quote_count % 2 == 0)) {
            // Found it
            found = true;
            break;
        }
    }

    if (found) {
        to.assign(from.begin(), iter);

        // Remove to from input
        from.erase(from.begin(), iter + 1);
    }

    return found;
}

std::string
demangle_type_name(const std::string& mangled)
{
    char* buffer;
    int status;

    buffer = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);

    if (status == 0) {
        std::string n(buffer);
        free(buffer);

        return n;
    } else {
        return std::string("demangle failure");
    }

    return std::string("unsupported");
}

std::string
escape(const char c)
{
    std::string esc;

    if (' ' <= c && c <= '~' && c != '\\' && c != '\'') {
        esc += c;
    } else {
        esc += '\\';

        switch (c) {
            case '\'':  esc += '\'';  break;
            case '\\': esc += '\\'; break;
            case '\t': esc += 't';  break;
            case '\r': esc += 'r';  break;
            case '\n': esc += 'n';  break;
            case '\0':  esc += '0';  break;
            default:
            char const* const hexdig = "0123456789ABCDEF";
            esc += 'x';
            esc += hexdig[c >> 4];
            esc += hexdig[c & 0xF];
        }
    }

    return esc;
}

std::string
escape_for_string(const char c)
{
    std::string esc;

    if (' ' <= c && c <= '~' && c != '\\' && c != '"') {
        esc += c;
    } else {
        esc += '\\';

        switch (c) {
            case '"':  esc += '"';  break;
            case '\\': esc += '\\'; break;
            case '\t': esc += 't';  break;
            case '\r': esc += 'r';  break;
            case '\n': esc += 'n';  break;
            default:
            char const* const hexdig = "0123456789ABCDEF";
            esc += 'x';
            esc += hexdig[c >> 4];
            esc += hexdig[c & 0xF];
        }
    }

    return esc;
}

std::string
escape(const std::string& s)
{
    std::string esc;

    for (const auto c : s) {
        esc += escape_for_string(c);
    }

    return esc;
}

ssize_t
send_file(int out_fd, int in_fd, off_t* offset, size_t count)
{
#if defined(__linux__)
    return ::sendfile(out_fd, in_fd, offset, count);
#elif defined(__APPLE__)
    off_t ofs = offset ? *offset : 0;
    off_t bytes = count;
    auto ret = ::sendfile(in_fd, out_fd, ofs, &bytes, NULL, 0);

    if (ret == -1) {
        return -1;
    }

    if (offset) {
        *offset += bytes;
    }

    return bytes;
#endif
}

bool
is_a_command(const std::filesystem::path& cmd)
{
    namespace fs = std::filesystem;

    bool ret = false;

    if (fs::exists(cmd)) {
        auto s = fs::status(cmd);
        auto t = s.type();
        auto p = s.permissions();

        ret =
            t == fs::file_type::regular
            &&
            (p & fs::perms::others_exec) != fs::perms::none
            ;
    }

    return ret;
}

std::string
find_cmd(const std::string& cmd, const std::string& env_var)
{
    namespace fs = std::filesystem;

    fs::path what = cmd;
    fs::path result;

    if (!env_var.empty()) {
        if (auto CMD = std::getenv(env_var.c_str())) {
            what = CMD;
        }
    }

    if (!what.is_absolute()) {
        if (auto PATH = std::getenv("PATH")) {
            for (const auto& dir : split(":", PATH)) {
                auto p = fs::path(dir) / what;

                if (is_a_command(p)) {
                    result = p;
                    break;
                }
            }
        }
    } else {
        result = what;
    }

    die_unless(fs::exists(result), "command not found: ", what);

    return result.string();
}

std::string
try_find_cmd(const std::string& cmd, const std::string& env_var)
{
    std::string result;

    try {
        result = find_cmd(cmd, env_var);
    } catch (...) {
        result.clear();
    }

    return result;
}

}
