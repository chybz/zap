#pragma once

namespace zap {

struct zapfile
{
    std::string name;
    std::string version;

    void load(const std::string& file = "Zapfile");
};

}
