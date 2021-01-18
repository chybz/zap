#include <zap/fetchers/curl.hpp>
#include <zap/utils.hpp>

namespace zap::fetchers {

curl::curl(const zap::config& cfg)
: fetcher(cfg)
{
    prog_.cmd = zap::find_cmd("curl");

    prog_.push_args({ "-LJO", "--progress-bar" });
}

curl::~curl()
{}

void
curl::download(
    const std::string& url,
    const std::string& dir
) const
{
    mkpath(dir);

    zap::call_in_directory(
        dir,
        [&] { prog_.run({ url }); }
    );
}

}
