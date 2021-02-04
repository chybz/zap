#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <ostream>

#include <zap/files.hpp>
#include <zap/types.hpp>

namespace zap {

enum class target_type
{
    bin,
    lib,
    mod,
    tst
};

const std::string&
to_string(target_type t);

std::ostream&
operator<<(std::ostream& os, target_type t);

struct target_deps
{
    string_set headers;
    string_set libs;
    string_set project_libs;

    strings ordered_libs;

    template <typename Container>
    void add_libs(const Container& c)
    { libs.insert(c.begin(), c.end()); }

    std::string to_string() const;
};

struct target
{
    std::string name;
    target_type type;
    std::string src_dir;
    std::string inc_dir;
    files public_headers;
    files private_headers;
    files sources;
    target_deps public_deps;
    target_deps private_deps;
    string_set pkg_deps;

    std::string to_string() const;

    bool is_bin() const;
    bool is_lib() const;
    bool is_mod() const;
    bool is_tst() const;

    bool has_public_dep(const std::string& dep) const;
    bool has_public_headers() const;
    bool has_public_header(const std::string& name) const;
    bool has_private_headers() const;
    bool has_private_header(const std::string& name) const;
    bool has_header(const std::string& name) const;
    bool has_source(const std::string& name) const;
    bool has_file(const std::string& name) const;
    bool has_sources() const;

    void normalize_libs();
    void normalize_libs(const string_set& pub, string_set& priv);
};

std::ostream&
operator<<(std::ostream& os, const target& t);

bool
operator==(const target& a, const target& b);

using targets = std::unordered_map<std::string, target>;
using target_type_dirs = std::unordered_map<target_type, std::string>;
using target_type_dir = target_type_dirs::value_type;

target_type_dir
target_src_dir(const string_map& sub_dirs, target_type t);

}
