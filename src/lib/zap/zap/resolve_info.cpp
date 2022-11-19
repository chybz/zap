#include <zap/resolve_info.hpp>

namespace zap {

bool
resolve_info::empty() const
{
    return
        to_install.empty()
        &&
        to_choose.empty()
        &&
        unresolved_headers.empty()
        &&
        ambiguous_headers.empty()
        ;
}

}
