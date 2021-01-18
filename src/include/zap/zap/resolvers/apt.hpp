#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <re2/re2.h>

#include <zap/toolchain.hpp>
#include <zap/resolver.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/pkg_configs.hpp>
#include <zap/cmake_configs.hpp>
#include <zap/dep_info.hpp>
#include <zap/pkg_items.hpp>

namespace zap::resolvers {

struct apt_context
{
    zap::pkg_items_map headers;
    zap::pkg_items_map pkg_config_names;
    zap::pkg_items_map cmake_names;
    zap::dep_info_map header_to_dep;
    zap::dep_infos file_headers;

    void merge(apt_context& other);
};

using apt_contexts = std::vector<apt_context>;

class apt : public zap::resolver
{
public:
    apt(const zap::toolchain& tc);
    virtual ~apt();

    zap::dep_info resolve(const std::string& header) const override;

    bool installed(const std::string& pkg) const override;

private:
    void load_contents();

    void resize(std::size_t size);

    void parse_contents(const std::string& file);

    void add_pkg_item(
        zap::pkg_items_map& m,
        const std::string& item,
        const std::string& pkgs
    );

    void make_deps(apt_context& ctx);

    void process_pkg_headers(
        apt_context& ctx,
        const std::string& pkg,
        const std::string& pcname = {}
    );

    void strip_pkg_headers(
        apt_context& ctx,
        const std::set<std::string>& inc_dirs,
        const std::string& pkg,
        const std::string& pcname
    );

    void load_installed();

    const zap::toolchain& tc_;
    zap::pkg_configs pc_;
    zap::cmake_configs cmc_;
    std::set<std::string> std_inc_dirs_;
    zap::string_set installed_;
    apt_contexts ctxs_;

    re2::RE2 inc_re_;
    re2::RE2 pc_re_;
    re2::RE2 cmake_re_;
    re2::RE2 pkg_re_;
};

}
