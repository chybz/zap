#pragma once

#include <zap/dependency.hpp>

namespace zap {

struct zapfile
{
    std::string name;
    std::string version;
    dependencies deps;

    void load(const std::string& file = "Zapfile");
};

}
