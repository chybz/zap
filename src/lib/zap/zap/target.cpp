#include <zap/target.hpp>
#include <zap/file_utils.hpp>

namespace zap {

const std::string&
to_string(target_type t)
{
    static std::unordered_map<target_type, std::string> m = {
        { target_type::bin, "bin" },
        { target_type::lib, "lib" },
        { target_type::mod, "mod" },
        { target_type::tst, "tst" }
    };

    return m.at(t);
}

std::ostream&
operator<<(std::ostream& os, target_type t)
{
    os << to_string(t);

    return os;
}

bool
target::has_public_headers() const
{ return !public_headers.empty(); }

bool
target::has_public_header(const std::string& name) const
{ return public_headers.has(name); }

bool
target::has_private_headers() const
{ return !private_headers.empty(); }

bool
target::has_private_header(const std::string& name) const
{ return private_headers.has(name); }

bool
target::has_header(const std::string& name) const
{ return has_public_header(name) || has_private_header(name); }

bool
target::has_source(const std::string& name) const
{ return sources.has(name); }

bool
target::has_file(const std::string& name) const
{ return has_header(name) || has_source(name); }

bool
target::has_sources() const
{ return !sources.empty(); }

target_type_dir
target_src_dir(const string_map& sub_dirs, target_type t)
{ return { t, cat_src_dir(sub_dirs, to_string(t)) }; }

}
