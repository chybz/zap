#pragma once

#include <string>

namespace zap::package {

class manifest
{
public:
    manifest();
    virtual ~manifest();

private:
    std::string dir_;
};

}
