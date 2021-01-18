#pragma once

#include <string>
#include <memory>
#include <vector>

#include <zap/toolchain.hpp>
#include <zap/dep_info.hpp>
#include <zap/types.hpp>

namespace zap {

class resolver
{
public:
    resolver(const std::string& name);
    virtual ~resolver();

    const std::string& name() const;

    virtual dep_info resolve(const std::string& header) const = 0;

    virtual bool installed(const std::string& pkg) const = 0;

private:
    std::string name_;
};

using resolver_ptr = std::unique_ptr<resolver>;
using resolver_ptrs = std::vector<resolver_ptr>;

void
make_resolvers(const toolchain& tc, resolver_ptrs& rps);

resolver_ptrs
make_resolvers(const toolchain& tc);

template <typename Resolver, typename... Args>
resolver_ptr
new_resolver(Args&&... args)
{ return std::make_unique<Resolver>(std::forward<Args>(args)...); }

}
