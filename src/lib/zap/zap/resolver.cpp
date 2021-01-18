#include <memory>
#include <unordered_map>

#include <zap/resolver.hpp>
#include <zap/resolvers/apt.hpp>
#include <zap/resolvers/conan.hpp>
#include <zap/utils.hpp>

namespace zap {

resolver::resolver(const std::string& name)
: name_(name)
{}

resolver::~resolver()
{}

const std::string&
resolver::name() const
{ return name_; }

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
void
make_resolvers(const toolchain& tc, resolver_ptrs& rps)
{
    if (tc.os_info().is_debian()) {
        auto rp = new_resolver<zap::resolvers::apt>(tc);
        rps.emplace_back(std::move(rp));
    }
}

resolver_ptrs
make_resolvers(const toolchain& tc)
{
    resolver_ptrs rps;

    make_resolvers(tc, rps);

    return rps;
}

}
