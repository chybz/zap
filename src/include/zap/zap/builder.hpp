#pragma once

#include <string>
#include <memory>

#include <zap/env.hpp>
#include <zap/archive_info.hpp>
#include <zap/types.hpp>
#include <zap/package/manifest.hpp>

namespace zap {

class builder_base
{
public:
    builder_base(
        const zap::env& e,
        const archive_info& ai,
        const strings& args = {}
    );

    virtual ~builder_base();

    virtual void configure() const = 0;
    virtual void build() const = 0;
    virtual void install(zap::package::manifest& pm) const = 0;

protected:
    const env& e_;
    const archive_info& ai_;
    const strings& args_;
};

using builder_ptr = std::unique_ptr<builder_base>;

class builder
{
public:
    builder(
        const zap::env& e,
        const archive_info& ai,
        const strings& args = {}
    );

    virtual ~builder();

    void configure() const;
    void build() const;
    void install(zap::package::manifest& pm) const;

private:
    builder_ptr bp_;
};


}
