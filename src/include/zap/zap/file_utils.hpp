#pragma once

#include <unordered_map>
#include <string>

#include <zap/types.hpp>

namespace zap {

enum class re_type
{
    src,
    hdr,
    src_or_hdr,
    shared_lib,
    static_lib,
    lib_link_name
};

const std::string&
re(re_type dt);

std::string
capture_re(re_type rt);

std::string
compose_re(const std::string& pre, re_type rt, const std::string& post);

bool
lib_is_shared(const std::string& lib);

bool
lib_is_static(const std::string& lib);

std::string
link_name(const std::string& name);

string_map
link_names(const strings& libs);

string_set
link_names_set(const strings& libs);

string_map
link_names_map(const strings& libs);

std::string
cat_src_dir(
    const zap::string_map&,
    const std::string& sub_dir_key
);

}
