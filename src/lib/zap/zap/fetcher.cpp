#include <fstream>

#include <httplib.h>

#include <zap/fetcher.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

fetcher::fetcher(const env_paths& ep)
: ep_(ep)
{}

fetcher::~fetcher()
{}

void
fetcher::download(
    const std::string& url,
    const std::string& dir,
    const std::string& filename
) const
{
    mkpath(dir);

    std::ofstream ofs(cat_file(dir, filename), std::ios::binary);



    die_unless(
        r.error.code == cpr::ErrorCode::OK,
        "failed to download ", url, ": ", r.error.code
    );
}

}
