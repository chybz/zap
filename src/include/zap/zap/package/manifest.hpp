#pragma once

namespace zap::package {

class manifest
{
public:
    manifest(const std::string& dir)
    virtual ~manifest();

private:
    std::string dir_;
};

}
