#include <zap/fetcher.hpp>

namespace zap::fetchers {

class curl : public zap::fetcher
{
public:
    curl(const zap::config& cfg);
    virtual ~curl();

    void download(
        const std::string& url,
        const std::string& dir
    ) const final;
};

}
