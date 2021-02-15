#include <zap/fetchers/curl.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::fetchers {

CURL*
curl_context::get() const
{ return p.get(); }

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

template <typename A>
void
set_curl_cb_opt(curl_context& ctx, CURLoption o, A&& a, const char* what)
{
    auto p = static_cast<void*>(std::addressof(a));

    set_curl_opt(ctx, o, p, what);
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
        auto pos = ctx.url.
    }

    return sz;
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
curl::curl(const zap::config& cfg)
: fetcher(cfg)
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
    set_curl_opt(ctx, CURLOPT_HEADERFUNCTION, curl_header_cb, "HEADER");
    set_curl_cb_opt(ctx, CURLOPT_HEADERDATA, ctx, "HEADER DATA");
    set_curl_opt(ctx, CURLOPT_WRITEFUNCTION, curl_write_cb, "WRITE");
    set_curl_cb_opt(ctx, CURLOPT_WRITEDATA, ctx, "WRITE DATA");

    auto cerr = curl_easy_perform(ctx.get());
}

}
