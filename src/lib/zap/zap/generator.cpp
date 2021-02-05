#include <zap/generator.hpp>

namespace zap {

generator::generator(const project& p)
: p_(p)
{}

generator::~generator()
{}

const project&
generator::p() const
{ return p_; }

}
