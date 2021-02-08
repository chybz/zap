#include <zap/generator.hpp>

namespace zap {

generator::generator(
    const toolchain& tc,
    const project& p
)
: tc_(tc),
p_(p)
{}

generator::~generator()
{}

const project&
generator::p() const
{ return p_; }

}
