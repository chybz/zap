#include <curl/curl.h>

#include <zap/fetcher.hpp>

namespace zap::fetchers {

struct curl_context;

class curl : public zap::fetcher
{
public:
    curl(const zap::env_paths& ep);
    virtual ~curl();

    void download(
        const std::string& url,
        const std::string& dir
    ) const final;

private:
    void download(curl_context& ctx) const;
};

}
