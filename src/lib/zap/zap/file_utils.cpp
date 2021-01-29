#include <regex>

#include <zap/file_utils.hpp>
#include <zap/utils.hpp>

namespace zap {

const std::string&
re(re_type rt)
{
    using re_map = std::unordered_map<re_type, std::string>;

    static re_map defaults = {
        { re_type::src, ".*\\.(?:c|cc|cpp|cxx)" },
        { re_type::hdr, ".*\\.(?:h|hh|hpp|hxx|inl|ipp)" },
        { re_type::src_or_hdr, ".*\\.(?:c|cc|cpp|cxx|h|hh|hpp|hxx|inl|ipp)" },
#if defined(_WIN32)
        { re_type::shared_lib, ".*\\.dll" },
        { re_type::static_lib, ".*\\.lib" },
        { re_type::lib_link_name, "(.*)\\.(?:lib|dll)" },
#elif defined(__APPLE__)
        { re_type::shared_lib, ".*\\.dylib" },
        { re_type::static_lib, ".*\\.a" },
#elif defined(__unix__)
        { re_type::shared_lib, ".*\\.so" },
        { re_type::static_lib, ".*\\.a" },
        { re_type::lib_link_name, "lib(.*)\\.(?:a|so|dylib).*" },
#endif
    };

    return defaults.at(rt);
}

std::string
capture_re(re_type rt)
{ return "(" + re(rt) + ")"; }

std::string
compose_re(const std::string& pre, re_type rt, const std::string& post)
{ return pre + re(rt) + post; }

bool
lib_is_shared(const std::string& lib)
{ return match(lib, re(re_type::shared_lib)); }

bool
lib_is_static(const std::string& lib)
{ return match(lib, re(re_type::static_lib)); }

std::string
link_name(const std::string& name)
{
    std::regex lnre(re(re_type::lib_link_name));
    std::smatch m;

    if (std::regex_match(name, m, lnre)) {
        return m[1];
    }

    return name;
}

string_map
link_names(const strings& libs)
{
    string_map link_names;

    for (const auto& l : libs) {
        link_names.try_emplace(link_name(l), l);
    }

    return link_names;
}

string_set
link_names_set(const strings& libs)
{
    string_set link_names;

    for (const auto& l : libs) {
        link_names.insert(link_name(l));
    }

    return link_names;
}

string_map
link_names_map(const strings& libs)
{
    string_map link_names;

    for (const auto& l : libs) {
        link_names.try_emplace(link_name(l), l);
    }

    return link_names;
}

std::string
cat_src_dir(
    const string_map& sub_dirs,
    const std::string& sub_dir_key
)
{ return cat_dir(sub_dirs.at("src"), sub_dirs.at(sub_dir_key)); }

}
