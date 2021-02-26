#include <curl/curl.h>

#include <memory>
#include <functional>
#include <fstream>

#include <re2/re2.h>

#include <zap/fetchers/curl.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::fetchers {

using curl_ptr = std::unique_ptr<CURL, std::function<void(CURL*)>>;

struct curl_context
{
    curl_ptr p;
    std::string url;
    std::string dir;
    re2::RE2 filename_re;
    std::string filename;
    std::string error;
    std::ofstream os;

    CURL* get() const
    { return p.get(); }
};

///////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
///////////////////////////////////////////////////////////////////////////////
curl_ptr
new_curl()
{
    curl_ptr cp(curl_easy_init(), curl_easy_cleanup);

    die_if(
        !cp,
        "failed to create a new libcurl session"
    );

    return cp;
}

template <typename A>
void
set_curl_opt(curl_context& ctx, CURLoption o, A&& a, const char* what)
{
    auto ret = curl_easy_setopt(ctx.get(), o, a);

    die_if(
        ret,
        "failed to set libcurl option ", what, ": ", ret
    );
}

size_t
curl_header_cb(void* ptr, size_t size, size_t nitems, void *userdata)
{
    auto& ctx = *static_cast<curl_context*>(userdata);
    std::size_t sz = size * nitems;
    std::string_view data{ static_cast<const char*>(ptr), sz };

    re2::StringPiece input(data);
    re2::StringPiece filename;

    if (re2::RE2::PartialMatch(input, ctx.filename_re, &filename)) {
        ctx.filename.assign(filename.data(), filename.size());
    }

    return sz;
}

size_t
curl_write_cb(void* ptr, size_t size, size_t nitems, void *userdata)
{
    auto& ctx = *static_cast<curl_context*>(userdata);
    std::size_t sz = size * nitems;
    std::string_view data{ static_cast<const char*>(ptr), sz };

    if (ctx.filename.empty()) {
        // No filename from headers, use url
        auto pos = ctx.url.rfind('/');

        if (pos == std::string::npos) {
            ctx.error = "failed to extract a valid filename from: " + ctx.url;
        } else {
            ctx.filename.assign(ctx.url.begin() + pos + 1, ctx.url.end());
        }
    }

    if (ctx.error.empty()) {
        if (!ctx.os.is_open()) {
            ctx.os.open(
                zap::cat_file(ctx.dir, ctx.filename),
                std::ios::binary
            );

            if (!ctx.os) {
                ctx.error = "failed to open file: " + ctx.filename;
            }
        }

        ctx.os << data;
    }

    return ctx.error.empty() ? sz : 0;
}

std::string
content_disposition_pat()
{
    /// Mmmmh, yeah.. Encoding and other niceties...
    std::string pat = "(?i)";
    pat += "Content-Disposition:.*filename\\*?=";
    pat += "['\"]?(?:UTF-\\d['\"]*)?([^;\\r\\n\"']*)['\"]?;?";

    return pat;
}

///////////////////////////////////////////////////////////////////////////////
//
// CURL fetcher
//
///////////////////////////////////////////////////////////////////////////////
curl::curl(const zap::env_paths& ep)
: fetcher(ep)
{
    auto ret = curl_global_init(CURL_GLOBAL_DEFAULT);

    die_unless(
         ret == 0,
        "failed to initialize libcurl: ", ret
    );
}

curl::~curl()
{
    curl_global_cleanup();
}

void
curl::download(
    const std::string& url,
    const std::string& dir
) const
{
    curl_context ctx{
        new_curl(),
        url,
        dir,
        content_disposition_pat()
    };

    download(ctx);
}

void
curl::download(curl_context& ctx) const
{
    zap::mkpath(ctx.dir);

    set_curl_opt(ctx, CURLOPT_URL, ctx.url.c_str(), "URL");
    set_curl_opt(ctx, CURLOPT_FOLLOWLOCATION, 1L, "FOLLOW LOCATION");
    set_curl_opt(ctx, CURLOPT_MAXREDIRS, 50L, "MAX REDIRS");
    set_curl_opt(ctx, CURLOPT_HEADERFUNCTION, curl_header_cb, "HEADER");
    set_curl_opt(ctx, CURLOPT_HEADERDATA, &ctx, "HEADER DATA");
    set_curl_opt(ctx, CURLOPT_WRITEFUNCTION, curl_write_cb, "WRITE");
    set_curl_opt(ctx, CURLOPT_WRITEDATA, &ctx, "WRITE DATA");

    auto cerr = curl_easy_perform(ctx.get());

    die_if(
        cerr != CURLE_OK,
        "failed to download ", ctx.url, ": ", cerr
    );
}

}
