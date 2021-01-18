#include <zap/archivers/zip.hpp>
#include <zap/utils.hpp>

namespace zap::archivers {

zip::zip(const config& cfg, const std::string& file)
: archiver_base(cfg, file)
{
    zip_.cmd = zap::find_cmd("zip");
    unzip_.cmd = zap::find_cmd("unzip");
}

zip::~zip()
{}

bool
zip::verify() const
{
    bool ok = false;

    try {
        unzip_.run({ "-qt", file_ });
        ok = true;
    } catch (...) {
    }

    return ok;
}

bool
zip::extract(const std::string& to) const
{
    bool ok = false;

    try {
        unzip_.run({ "-q", "-d", to, file_ });
        ok = true;
    } catch (...) {
    }

    return ok;
}

}
