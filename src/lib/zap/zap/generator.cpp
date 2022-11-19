#include <zap/generator.hpp>

namespace zap {

generator::generator(
    const zap::env& e,
    const project& p
)
: e_(e),
p_(p)
{}

generator::~generator()
{}

const project&
generator::p() const
{ return p_; }

}
