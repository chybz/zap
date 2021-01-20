#include <zap/builder.hpp>
#include <zap/builders/cmake.hpp>
#include <zap/builders/autotools.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

builder_base::builder_base(const toolchain& tc, const archive_info& ai)
: tc_(tc),
ai_(ai)
{}

builder_base::~builder_base()
{}

builder::builder(const toolchain& tc, const archive_info& ai)
{
    if (file_exists(cat_file(ai.source_dir, "CMakeLists.txt"))) {
        bp_ = std::make_unique<zap::builders::cmake>(tc, ai);
    } else if (file_exists(cat_file(ai.source_dir, "autogen.sh"))) {
        bp_ = std::make_unique<zap::builders::autotools>(tc, ai);
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
builder::install() const
{ bp_->install(); }

}
