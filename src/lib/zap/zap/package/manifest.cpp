#include <zap/package/manifest.hpp>

namespace zap::package {

manifest::manifest(const std::string& dir)
: dir_(dir)
{}

manifest::~manifest()
{}

}
