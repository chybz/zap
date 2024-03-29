#include <fstream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <zap/fetcher.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/url.hpp>

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

    auto file = cat_file(dir, filename);

    std::ofstream ofs(file, std::ios::binary);

    die_if(!ofs, "failed to open file: ", file);

    zap::url u(url);

    die_unless(u.parsed, "invalid url: ", url);

    httplib::Client c(u.host());

    c.set_follow_location(true);

    // TOFIX: need to find a better way to fix this
    c.enable_server_certificate_verification(false);

    auto res = c.Get(
        u.uri,
        [&] (const char* data, size_t size) {
            ofs.write(data, size);

            die_if(!ofs, "failed write file: ", file);

            return true;
        }
    );

    die_if(
        !res,
        "HTTP error while downloading file: ",
        file,
        httplib::to_string(res.error())
    );
}

void
fetcher::download_repo_archive(
    const std::string& repo,
    const std::string& ref,
    const std::string& dir
) const
{}

}
