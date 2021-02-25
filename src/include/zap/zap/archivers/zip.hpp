#pragma once

#include <zap/archiver.hpp>
#include <zap/prog.hpp>

namespace zap::archivers {

class zip : public zap::archiver_base
{
public:
    zip(const zap::env_paths& ep, const std::string& file);
    virtual ~zip();

    bool verify() const final;
    bool extract(const std::string& to) const final;

private:
    zap::prog zip_;
    zap::prog unzip_;
};

}
