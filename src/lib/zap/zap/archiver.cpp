#include <zap/archiver.hpp>
#include <zap/archivers/zip.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

archiver_base::archiver_base(const config& cfg, const std::string& file)
: cfg_(cfg),
file_(file)
{}

archiver_base::~archiver_base()
{}

archiver::archiver(const config& cfg, const std::string& file)
{
    if (imatch(file, ".*\\.zip")) {
        ap_ = std::make_unique<zap::archivers::zip>(cfg, file);
    } else {
        die("unknown archive format: ", file);
    }
}

archiver::~archiver()
{}

bool
archiver::verify()
{ return ap_->verify(); }

bool
archiver::extract(const std::string& to)
{ return ap_->extract(to); }

}
