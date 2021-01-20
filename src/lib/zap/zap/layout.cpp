#include <memory>

#include <zap/layout.hpp>
#include <zap/layouts/app.hpp>
#include <zap/layouts/cbuild.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

using layout_ptr = std::unique_ptr<layout>;

layout::layout(
    const std::string& label,
    const std::string& project_dir
)
: label_(label),
project_dir_(project_dir)
{}

layout::~layout()
{}

const std::string&
layout::label() const
{ return label_; }

template <typename Layout, typename... Args>
layout_ptr
new_layout(Args&&... args)
{ return std::make_unique<Layout>(std::forward<Args>(args)...); }

template <typename Layout>
bool
try_layout(layout_ptr& lp, const std::string& dir)
{
    bool detected = false;
    auto tlp = new_layout<Layout>(dir);

    if ((detected = tlp->detect())) {
        lp = std::move(tlp);
    }

    return detected;
}

template <typename Layout, typename... Layouts>
bool
detect_layout(layout_ptr& lp, const std::string& dir)
{
    bool detected = false;

    if ((detected = try_layout<Layout>(lp, dir))) {
        return true;
    } else {
        if constexpr (sizeof...(Layouts) > 0) {
            detected = detect_layout<Layouts...>(lp, dir);
        }
    }

    return detected;
}

void
make_layout(layout_ptr& lp, const std::string& dir)
{
    bool found = detect_layout<
        zap::layouts::app,
        zap::layouts::cbuild
    >(lp, dir);

    die_unless(
        found,
        "no suitable project layout detected"
    );

    log("project layout is: ", lp->label());
}

layout&
get_layout(const std::string& dir)
{
    static layout_ptr lp = nullptr;

    if (!lp) {
        make_layout(lp, dir);
    }

    return *lp;
}

}
