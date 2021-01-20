#pragma once

#include <string>
#include <vector>
#include <unordered_map>
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

struct target
{
    std::string name;
    target_type type;
    std::string src_dir;
    std::string inc_dir;
    files public_headers;
    files private_headers;
    files sources;
    string_vector_map public_header_deps;
    string_vector_map private_header_deps;
    string_vector_map project_lib_deps;
    string_vector_map public_pkg_deps;
    string_vector_map private_pkg_deps;

    bool has_public_headers() const;
    bool has_public_header(const std::string& name) const;
    bool has_private_headers() const;
    bool has_private_header(const std::string& name) const;
    bool has_header(const std::string& name) const;
    bool has_source(const std::string& name) const;
    bool has_file(const std::string& name) const;
    bool has_sources() const;
};

using targets = std::vector<target>;
using target_type_dirs = std::unordered_map<target_type, std::string>;
using target_type_dir = target_type_dirs::value_type;

target_type_dir
target_src_dir(const string_map& sub_dirs, target_type t);

}
