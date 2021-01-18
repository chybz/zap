#include <zap/archive_utils.hpp>
#include <zap/utils.hpp>

namespace zap {

archive_util::archive_util(const std::string& file)
: file_(file),
a_(archive_read_new())
{
    archive_read_support_filter_all(a_);
    archive_read_support_format_raw(a_);

    check(
        archive_read_open_filename(a_, file_.c_str(), buffer_size_),
        "open archive"
    );
}

archive_util::~archive_util()
{
    if (a_) {
        archive_read_free(a_);
        a_ = nullptr;
    }
}

std::string_view
archive_util::make_line_block(const buffer_type& b)
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

void
archive_util::check(int r, const char* what) const
{
    die_unless(
        r == ARCHIVE_OK,
        what, " failed: ", archive_error_string(a_)
    );
}

}
