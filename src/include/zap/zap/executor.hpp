#pragma once

#include <thread>
#include <future>
#include <vector>

#include <taskflow/taskflow.hpp>

namespace zap {

using executor = tf::Executor;

std::size_t
adjust_par_level(const executor& exec, std::size_t par_level);

template <typename Context>
struct async_contexts
{
    using return_type = Context&;
    using contexts = std::vector<Context>;

    async_contexts(std::size_t size)
    : ctxs(size)
    {}

    ~async_contexts()
    {}

    template <typename Callable, typename... Args>
    void call(std::size_t index, Callable& cb, Args&&... args)
    { cb(ctxs[index], std::forward<Args>(args)...); }

    return_type merge()
    {
        while (ctxs.size() > 1) {
            ctxs.front().merge(ctxs.back());
            ctxs.pop_back();
        }

        return ctxs.front();
    }

    contexts ctxs;
};

template <>
struct async_contexts<void>
{
    using return_type = void;

    async_contexts(std::size_t size)
    {}

    ~async_contexts()
    {}

    template <typename Callable, typename... Args>
    void call(std::size_t index, Callable& cb, Args&&... args)
    { cb(index, std::forward<Args>(args)...); }

    void merge()
    {}
};

template <
    typename Callable,
    typename Context = void
>
class async_pool
{
public:
    using contexts_type = async_contexts<Context>;
    using return_type = typename contexts_type::return_type;

    async_pool(
        zap::executor& exec,
        Callable cb,
        std::size_t par_level = 0
    )
    : exec_(exec),
    cb_(std::move(cb)),
    par_level_(adjust_par_level(exec, par_level)),
    actxs_(par_level_)
    {}

    virtual ~async_pool()
    { wait(); }

    template <typename... Args>
    void async(Args&&... args)
    {
        if (futures_.size() == par_level_) {
            wait_futures();
        }

        auto index = futures_.size();

        auto f = exec_.async(
            [&, index, ...args = std::forward<Args>(args)] {
                actxs_.call(index, cb_, args...);
            }
        );

        futures_.emplace_back(std::move(f));
    }


    return_type wait()
    {
        wait_futures();

        return actxs_.merge();
    }

private:
    using future_type = std::future<void>;
    using futures = std::vector<future_type>;

    void wait_futures()
    {
        for (const auto& f : futures_) {
            f.wait();
        }

        futures_.clear();
    }

    zap::executor& exec_;
    Callable cb_;
    std::size_t par_level_;
    async_contexts<Context> actxs_;
    futures futures_;
};

}
