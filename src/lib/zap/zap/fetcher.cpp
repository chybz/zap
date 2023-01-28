#include <fstream>

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

    auto u = parse_url(url);

    die_unless(u.parsed, "invalid url: ", url);

    httplib::Client c(u.hostname);

    auto res = c.Get(u.uri,
        [&] (const char* data, size_t size) {
            ofs.write(data, size);

            die_if(!ofs, "failed write file: ", file);

            return true;
        }
    );
}

}
