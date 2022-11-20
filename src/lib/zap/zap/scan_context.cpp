#include <zap/scan_context.hpp>

namespace zap {

void
scan_context::merge(scan_context& other)
{ deps.merge(other.deps); }

}
