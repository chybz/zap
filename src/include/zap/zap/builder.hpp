#pragma once

#include <string>
#include <memory>

#include <zap/env.hpp>
#include <zap/archive_info.hpp>

namespace zap {

class builder_base
{
public:
    builder_base(const zap::env& e, const archive_info& ai);
    virtual ~builder_base();

    virtual void configure() const = 0;
    virtual void build() const = 0;
    virtual const std::string& install() const = 0;

protected:
    const env& e_;
    const archive_info& ai_;
};

using builder_ptr = std::unique_ptr<builder_base>;

class builder
{
public:
    builder(const zap::env& e, const archive_info& ai);
    virtual ~builder();

    void configure() const;
    void build() const;
    const std::string& install() const;

private:
    builder_ptr bp_;
};


}
