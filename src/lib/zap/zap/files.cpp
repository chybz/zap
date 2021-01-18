#include <zap/files.hpp>
#include <zap/utils.hpp>

namespace zap {

void
add_files(files& f, const std::string& dir, const std::string& re)
{
    if (!directory_exists(dir)) {
        return;
    }

    for (auto&& lf : find_files(dir, re)) {
        f.add(std::move(lf));
    }
}

}
