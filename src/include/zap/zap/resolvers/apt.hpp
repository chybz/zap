#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <re2/re2.h>

#include <zap/env.hpp>
#include <zap/resolver.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>

namespace zap::resolvers {

class apt : public zap::resolver
{
public:
    apt(const zap::env& e);
    virtual ~apt();

private:
    void load_contents();

    void parse_contents(const std::string& file);

    void add_pkg_item(
        zap::pkg_items_map& m,
        const std::string& item,
        const std::string& pkgs,
        string_set_map* rev = nullptr
    );

    void load_installed();

    re2::RE2 inc_re_;
    re2::RE2 pc_re_;
    re2::RE2 cmake_re_;
    re2::RE2 pkg_re_;
    re2::RE2 lib_re_;
};

}
