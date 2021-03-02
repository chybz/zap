#pragma once

#include <memory>

namespace zap::db {

struct storage_base
{
    storage_base();
    virtual ~storage_base();
};

using storage_ptr = std::unique_ptr<storage_base>;

}
