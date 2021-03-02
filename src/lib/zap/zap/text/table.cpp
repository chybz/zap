#include <zap/text/table.hpp>

namespace zap::text {

table::~table()
{}

void
table::pre()
{
    t_.format()
        .corner_top_left("")
        .corner_top_right("")
        .corner_bottom_left("")
        .corner_bottom_right("")
        .border_top("")
        .border_bottom("")
        .border_left("")
        .border_right("")
        ;
}

void
table::post()
{
    if (rows_ >= 1) {
        t_[1].format().border_top("-");
    }
}

}
