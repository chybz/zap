#pragma once

#include <string>

namespace zap {

struct archive_info
{
    std::string url;
    std::string file;
    std::string dir;
    std::string source_dir;
    std::string temp_dir;
    std::string name;
    std::string version;
};

}
