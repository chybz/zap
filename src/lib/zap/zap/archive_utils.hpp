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

template <
    typename Context = void
>
class archive_util
{
public:
    archive_util(
        const std::string& file,
        executor& exec
    )
    : file_(file),
    exec_(exec),
    a_(archive_read_new())
    {
        archive_read_support_filter_all(a_);
        archive_read_support_format_raw(a_);

        check(
            archive_read_open_filename(a_, file_.c_str(), buffer_size_),
            "open archive"
        );
    }

    virtual ~archive_util()
    {
        if (a_) {
            archive_read_free(a_);
            a_ = nullptr;
        }
    }

    template <typename Callable>
    void
    for_each_line_block(Callable&& cb, Context& ctx)
    {
        // Only one header in raw mode
        check(
            archive_read_next_header(a_, &ae_),
            "read archive header"
        );

        async_pool<Callable, Context> ap(exec_, cb);

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

        ctx.merge(ap.wait());
    }

private:
    using buffer_type = std::vector<char>;

    std::string_view make_line_block(const buffer_type& b)
    {
        std::string_view lines(b.data(), b.size());

        auto pos = lines.rfind('\n');

        if (pos != lines.npos) {
            lines.remove_suffix(lines.size() - pos);

            if (lines.back() == '\r') {
                lines.remove_suffix(1);
            }

            remaining_.clear();
        } else {
            lines = {};
        }

        remaining_.insert(remaining_.end(), b.begin() + lines.size(), b.end());

        return lines;
    }

    void check(int r, const char* what) const
    {
        die_unless(
            r == ARCHIVE_OK,
            what, " failed: ", archive_error_string(a_)
        );
    }

    std::size_t buffer_size_ = 1024*1024;
    std::string file_;
    executor& exec_;
    archive* a_ = nullptr;
    archive_entry* ae_ = nullptr;
    buffer_type remaining_;
};

}
