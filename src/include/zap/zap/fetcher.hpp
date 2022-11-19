#pragma once

#include <string>
#include <memory>

#include <zap/env_paths.hpp>

namespace zap {

class fetcher
{
public:
    fetcher(const env_paths& ep);
    virtual ~fetcher();

    virtual void download(
        const std::string& url,
        const std::string& dir
    ) const = 0;

protected:
    const env_paths& ep_;
};

using fetcher_ptr = std::unique_ptr<fetcher>;

template <typename Fetcher, typename... Args>
fetcher_ptr
new_fetcher(Args&&... args)
{ return std::make_unique<Fetcher>(std::forward<Args>(args)...); }

}
