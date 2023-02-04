#include <string>

#include <re2/re2.h>

#include <zap/zapfile.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/url.hpp>

namespace zap {

void
zapfile::load(const std::string& file)
{
    die_unless(file_exists(file), "invalid file: ", file);

    YAML::Node c = YAML::LoadFile(file);

    load_deps(c);
}

void
zapfile::load_deps(const YAML::Node& c)
{
    if (!c["depends"]) {
        return;
    }

    die_unless(c["depends"].IsSequence(), "depends is not a sequence");

    re2::RE2 depexpr("(?:(\\w+):)?([^@]+)(?:@(.+))?");

    std::string id;
    std::string spec;
    std::string version;

    for (const auto& dep : c["depends"]) {
        std::string s;
        dependency d;

        if (dep.IsScalar()) {
            s = dep.as<std::string>();
        } else if (dep.IsMap()) {
            die_if(dep.size() != 1, "dependency map size is not 1: ", dep);

            auto it = dep.begin();
            s = it->first.as<std::string>();
            auto opts = it->second;

            die_unless(
                opts.IsMap(), "dependency options is not a map: ",
                opts
            );

            load_strings(opts, "configure", d.opts);
            load_strings(opts, "build", d.opts);
            load_strings(opts, "install", d.opts);
        } else {
            die("dependency is not a scalar or map: ", dep);
        }

        if (re2::RE2::FullMatch(s, depexpr, &id, &spec, &version)) {
            set_remote(id, spec, version, d);
        } else if (re2::RE2::FullMatch(s, urlexpr, &scheme)) {
            std::cout
                << "dep direct URL scheme=" << scheme
                << std::endl;
        } else {
            die("invalid dependency: ", s);
        }
    }
}

void
zapfile::set_remote(
    const std::string& id,
    const std::string& spec,
    const std::string& version,
    dependency& d
)
{}

void
zapfile::set_remote_repository(
    const std::string& id,
    const std::string& spec,
    const std::string& version,
    dependency& d
)
{
    url u;

    if (id.empty()) {
        t = from_string("github");
    } else {
        t = from_string(type);
    }


}

void
zapfile::load_strings(
    const YAML::Node& n,
    const std::string& key,
    strings_map& m
)
{
    if (!n[key]) {
        return;
    }

    die_unless(
        n[key].IsSequence(),
        "options for '", key, "' is not a sequence: ",
        n[key]
    );

    auto& l = m[key];

    for (const auto& p : n[key]) {
        auto v = p.as<std::string>();
        l.emplace_back(std::move(v));
    }
}

}
