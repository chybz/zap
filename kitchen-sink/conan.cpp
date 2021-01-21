#include <zap/resolvers/conan.hpp>
#include <zap/file_utils.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/types.hpp>
#include <zap/graph.hpp>

namespace zap::resolvers {

namespace detail {

template <typename Callable>
void
walk_packages(
    const std::string& base,
    zap::strings& stack,
    Callable&& cb
)
{
    auto cur_dir = zap::cat_dir(base, zap::join("/", stack));

    for (auto&& d : zap::find_dirs(cur_dir)) {
        stack.emplace_back(std::move(d));

        if (stack.size() == 4) {
            // e.g.: boost/1.71.0/conan/stable
            auto pkg_dir = zap::cat_dir(
                base, zap::join("/", stack), "package"
            );

            for (const auto& id : zap::find_dirs(pkg_dir)) {
                conan_pkg cp{ stack[0], stack[1], stack[2], stack[3], id };

                cb(cp, zap::cat_dir(pkg_dir, id));
            }

            stack.clear();
        } else {
            walk_packages(base, stack, std::forward<Callable>(cb));
        }
    }
}

}

template <typename Callable>
void
walk_packages(const std::string& base, Callable&& cb)
{
    strings stack;

    detail::walk_packages(base, stack, std::forward<Callable>(cb));
}

conan::conan(const zap::toolchain& tc)
: resolver("conan"),
tc_(tc),
conan_{ zap::find_cmd("conan") }
{
    scan_packages();
}

conan::~conan()
{}

zap::dep_info
conan::resolve(const std::string& header) const
{
    return {};
}

void
conan::scan_packages()
{
    auto sp = conan_.get_line({ "config", "get", "storage.path" });

    if (!zap::directory_exists(sp)) {
        return;
    }

    walk_packages(
        sp,
        [&](auto& pkg, const auto& dir) {
            scan_package(pkg, dir);
        }
    );
}

void
conan::scan_package(conan_pkg& cp, const std::string& dir)
{
    auto lib_dir = zap::cat_dir(dir, "lib");

    auto shared_libs = zap::find_files(
        lib_dir,
        zap::re(zap::re_type::shared_lib)
    );

    auto static_libs = zap::find_files(
        lib_dir,
        zap::re(zap::re_type::static_lib)
    );

    if (shared_libs.empty() && static_libs.empty()) {
        // Header only
    } else if (static_libs.empty()) {
        add_shared(cp, shared_libs, dir);
    } else {
        add_static(cp, static_libs, dir);
    }
}

void
conan::add_shared(
    conan_pkg& cp,
    const zap::strings& libs,
    const std::string& dir
)
{
    // Cases:
    // - single library, simple (e.g.: zlib, bzip2)
    // - multiple libraries with direct dependencies forming a unique chain,
    //   simple (e.g.: OpenSSL: libssl depends on libcrypto)
    // - multiple libraries like in Boost or Poco
    //   1. form headers to lib map using 2 level directories
    //   2. find a pattern to join levels and have a lib name:
    //      libDIR1_DIR2, libDIR1DIR2 should be sufficient
    //   3. look for inter-lib dependencies using ldd/nm
    auto inc_dir = zap::cat_dir(dir, "include");
    auto lib_dir = zap::cat_dir(dir, "lib");
    auto lns = zap::link_names(libs);
    auto accepted = zap::link_names_set(libs);
    zap::graph g(lns.keys());

    for (const auto& ln : lns.keys()) {
        auto deps = tc_.local_lib_deps(
            zap::cat_file(lib_dir, lns[ln]),
            accepted
        );

        for (const auto& d : deps) {
            g.add_edge(ln, d);
        }
    }

    if (g.is_tree()) {
        auto ordered = g.ordered();

        add_single(cp, ordered, inc_dir);
    }
}

void
conan::add_static(
    conan_pkg& cp,
    const zap::strings& libs,
    const std::string& dir
)
{}

void
conan::add_single(
    conan_pkg& cp,
    zap::strings& libs,
    const std::string& inc_dir
)
{
    zap::dep d{ cp.name };

    d.headers.push_back(
        zap::find_files(
            inc_dir,
            zap::re(zap::re_type::hdr)
        )
    );

    d.libs = std::move(libs);

    cp.components.emplace_back(std::move(d));
}

}
