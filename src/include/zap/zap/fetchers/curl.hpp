#include <curl/curl.h>

#include <memory>
#include <functional>
#include <fstream>

#include <re2/re2.h>

#include <zap/fetcher.hpp>

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

    CURL* get() const;
};

class curl : public zap::fetcher
{
public:
    curl(const zap::config& cfg);
    virtual ~curl();

    void download(
        const std::string& url,
        const std::string& dir
    ) const final;

private:
    void download(curl_context& ctx) const;
};

}
