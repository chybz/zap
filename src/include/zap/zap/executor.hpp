#pragma once

#include <numeric>
#include <thread>
#include <mutex>
#include <future>
#include <vector>
#include <memory>

#include <taskflow/taskflow.hpp>

namespace zap {

using executor = tf::Executor;
using executor_ptr = std::unique_ptr<executor>;

std::size_t
adjust_par_level(const executor& exec, std::size_t par_level);

struct async_state
{
    async_state(std::size_t size)
    : size_(size),
    positions_(size_)
    {
        std::iota(positions_.begin(), positions_.end(), 0);
    }

    void one_done(std::size_t pos)
    {
        {
            std::lock_guard<std::mutex> g(m_);

            positions_.emplace_back(pos);
        }

        cv_.notify_one();
    }

    std::size_t wait_avail()
    {
        std::size_t pos;

        {
            std::unique_lock<std::mutex> lk(m_);

            cv_.wait(lk, [&]{ return !positions_.empty(); });

            pos = positions_.back();
            positions_.pop_back();
        }

        return pos;
    }

    void wait_all()
    {
        std::unique_lock<std::mutex> lk(m_);

        cv_.wait(lk, [&]{ return positions_.size() == size_; });
    }

    std::size_t size_;
    std::vector<std::size_t> positions_;
    std::mutex m_;
    std::condition_variable cv_;
};

template <typename Context>
struct async_contexts
{
    using return_type = Context&;
    using contexts = std::vector<Context>;

    async_contexts(std::size_t size, async_state& s)
    : ctxs_(size),
    s_(s)
    {}

    ~async_contexts()
    {}

    template <typename Callable, typename... Args>
    void call(std::size_t index, Callable& cb, Args&&... args)
    {
        cb(ctxs_[index], std::forward<Args>(args)...);
        s_.one_done(index);
    }

    return_type merge()
    {
        while (ctxs_.size() > 1) {
            ctxs_.front().merge(ctxs_.back());
            ctxs_.pop_back();
        }

        return ctxs_.front();
    }

    contexts ctxs_;
    async_state& s_;
};

template <>
struct async_contexts<void>
{
    using return_type = void;

    async_contexts(std::size_t size, async_state& s)
    : s_(s)
    {}

    ~async_contexts()
    {}

    template <typename Callable, typename... Args>
    void call(std::size_t index, Callable& cb, Args&&... args)
    {
        cb(index, std::forward<Args>(args)...);
        s_.one_done(index);
    }

    void merge()
    {}

    async_state& s_;
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
    s_(par_level_),
    actxs_(par_level_, s_)
    {}

    virtual ~async_pool()
    { wait(); }

    template <typename... Args>
    void async(Args&&... args)
    {
        auto index = s_.wait_avail();

        exec_.silent_async(
            [&, index, ...args = std::forward<Args>(args)]() mutable {
                actxs_.call(index, cb_, args...);
            }
        );
    }

    return_type wait()
    {
        s_.wait_all();

        return actxs_.merge();
    }

private:
    zap::executor& exec_;
    Callable cb_;
    std::size_t par_level_;
    async_state s_;
    async_contexts<Context> actxs_;
};

}
