#include <zap/fetchers/curl.hpp>
#include <zap/prog.hpp>
#include <zap/utils.hpp>

namespace zap::fetchers {

curl::curl(const zap::config& cfg)
: fetcher(cfg)
{
    prog_ = zap::find_prog("curl");
    prog_.args = { "-LJO", "--progress-bar" };
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
