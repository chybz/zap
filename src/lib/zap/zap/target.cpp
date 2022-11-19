#include <sstream>

#include <zap/target.hpp>
#include <zap/file_utils.hpp>
#include <zap/utils.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// Target type
//
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
//
// Target deps
//
///////////////////////////////////////////////////////////////////////////////
std::string
target_deps::to_string() const
{
    std::ostringstream os;

    os << join(", ", ordered_libs);

    return os.str();
}

///////////////////////////////////////////////////////////////////////////////
//
// Target
//
///////////////////////////////////////////////////////////////////////////////
bool
target::is_bin() const
{ return type == target_type::bin; }

bool
target::is_lib() const
{ return type == target_type::lib; }

bool
target::is_mod() const
{ return type == target_type::mod; }

bool
target::is_tst() const
{ return type == target_type::tst; }

std::string
target::to_string() const
{
    std::ostringstream os;

    os
        << "name: " << name << "\n"
        << "type: " << type << "\n"
        << "public deps: " << public_deps.to_string() << "\n"
        << "private deps: " << private_deps.to_string()
        ;

    return os.str();
}

bool
target::has_public_dep(const std::string& dep) const
{
    return
        public_deps.project_libs.contains(dep)
        ||
        public_deps.libs.contains(dep)
        ;
}

bool
target::has_public_headers() const
{ return !public_headers.empty(); }

bool
target::has_public_header(const std::string& name) const
{ return public_headers.contains(name); }

bool
target::has_private_headers() const
{ return !private_headers.empty(); }

bool
target::has_private_header(const std::string& name) const
{ return private_headers.contains(name); }

bool
target::has_header(const std::string& name) const
{ return has_public_header(name) || has_private_header(name); }

bool
target::has_source(const std::string& name) const
{ return sources.contains(name); }

bool
target::has_file(const std::string& name) const
{ return has_header(name) || has_source(name); }

bool
target::has_sources() const
{ return !sources.empty(); }

void
target::normalize_libs()
{
    normalize_libs(public_deps.libs, private_deps.libs);
    normalize_libs(public_deps.project_libs, private_deps.project_libs);
}

void
target::normalize_libs(const string_set& pub, string_set& priv)
{
    // Remove private libs already present in public libs
    for (auto it = priv.begin(); it != priv.end(); ) {
        if (pub.contains(*it)) {
            it = priv.erase(it);
        } else {
            ++it;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
///////////////////////////////////////////////////////////////////////////////
target_type_dir
target_src_dir(const string_map& sub_dirs, target_type t)
{ return { t, cat_src_dir(sub_dirs, to_string(t)) }; }

std::ostream&
operator<<(std::ostream& os, const target& t)
{
    os << t.to_string();

    return os;
}

bool
operator==(const target& a, const target& b)
{ return a.name == b.name && a.type == b.type; }

}
