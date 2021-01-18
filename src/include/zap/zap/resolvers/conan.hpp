#pragma once

#include <string>
#include <vector>

#include <zap/toolchain.hpp>
#include <zap/resolver.hpp>
#include <zap/prog.hpp>
#include <zap/dep.hpp>

namespace zap::resolvers {

struct conan_pkg
{
    std::string name;
    std::string version;
    std::string user;
    std::string channel;
    std::string id;

    zap::deps components;
};

using conan_pkgs = std::vector<conan_pkg>;

class conan : public zap::resolver
{
public:
    conan(const zap::toolchain& tc);
    virtual ~conan();

    zap::dep_info resolve(const std::string& header) const override;

private:
    void scan_packages();

    void scan_package(
        conan_pkg& cp,
        const std::string& dir
    );

    void add_shared(
        conan_pkg& cp,
        const zap::strings& libs,
        const std::string& dir
    );

    void add_static(
        conan_pkg& cp,
        const zap::strings& libs,
        const std::string& dir
    );

    void add_single(
        conan_pkg& cp,
        zap::strings& libs,
        const std::string& inc_dir
    );

    const zap::toolchain& tc_;
    zap::prog conan_;
    zap::dep_map dm_;
    conan_pkgs pkgs_;
};

}
