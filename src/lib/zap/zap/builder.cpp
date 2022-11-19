#include <zap/builder.hpp>
#include <zap/builders/cmake.hpp>
#include <zap/builders/autotools.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

builder_base::builder_base(
    const zap::env& e,
    const archive_info& ai,
    const strings& args
)
: e_(e),
ai_(ai),
args_(args)
{}

builder_base::~builder_base()
{}

builder::builder(
    const zap::env& e,
    const archive_info& ai,
    const strings& args
)
{
    if (file_exists(cat_file(ai.source_dir, "CMakeLists.txt"))) {
        bp_ = std::make_unique<zap::builders::cmake>(e, ai, args);
    } else if (file_exists(cat_file(ai.source_dir, "configure"))) {
        bp_ = std::make_unique<zap::builders::autotools>(e, ai, args);
    } else {
        die("unknown build system in dir: ", ai.source_dir);
    }
}

builder::~builder()
{}

void
builder::configure() const
{ bp_->configure(); }

void
builder::build() const
{ bp_->build(); }

void
builder::install(zap::package::manifest& pm) const
{ bp_->install(pm); }

}
