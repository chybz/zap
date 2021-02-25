#pragma once

#include <string>
#include <memory>

#include <zap/env_paths.hpp>

namespace zap {

class archiver_base
{
public:
    archiver_base(const env_paths& ep, const std::string& file);
    virtual ~archiver_base();

    virtual bool verify() const = 0;
    virtual bool extract(const std::string& to) const = 0;

protected:
    const env_paths& ep_;
    std::string file_;
};

using archiver_ptr = std::unique_ptr<archiver_base>;

class archiver
{
public:
    archiver(const env_paths& ep, const std::string& file);
    virtual ~archiver();

    bool verify();
    bool extract(const std::string& to);

private:
    archiver_ptr ap_;
};

}
