#pragma once

#include <zap/layouts/app.hpp>

namespace zap::layouts {

class cbuild : public app
{
public:
    cbuild(const std::string& project_dir);

    virtual ~cbuild();
};

}
