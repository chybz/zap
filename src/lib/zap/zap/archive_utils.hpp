#pragma once

#include <archive.h>
#include <archive_entry.h>

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <thread>
#include <future>

#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/executor.hpp>

namespace zap {

class archive_util
{
public:
    archive_util(const std::string& file);
    virtual ~archive_util();

    template <typename Callable>
    void
    for_each_line_block(
        zap::executor& exec,
        Callable&& cb,
        std::size_t par_level = 0
    )
    {
        // Only one header in raw mode
        check(
            archive_read_next_header(a_, &ae_),
            "read archive header"
        );

        async_pool<Callable> ap(exec, cb, par_level);

        remaining_.reserve(buffer_size_);

        for ( ; ; ) {
            buffer_type new_data;
            new_data.resize(buffer_size_);

            std::copy(remaining_.begin(), remaining_.end(), new_data.begin());

            auto read = archive_read_data(
                a_,
                new_data.data() + remaining_.size(),
                new_data.size() - remaining_.size()
            );

            new_data.resize(remaining_.size() + read);

            if (read < 0) {
                die("bad read", archive_error_string(a_));
            }

            if (read == 0) {
                break;
            }

            auto lines = make_line_block(new_data);

            if (!lines.empty()) {
                ap.async(std::move(new_data));
            }
        }

        ap.wait();
    }

private:
    using buffer_type = std::vector<char>;

    std::string_view make_line_block(const buffer_type& b);

    void check(int r, const char* what) const;

    std::size_t buffer_size_ = 1024*1024;
    std::string file_;
    archive* a_ = nullptr;
    archive_entry* ae_ = nullptr;
    buffer_type remaining_;
};

}
