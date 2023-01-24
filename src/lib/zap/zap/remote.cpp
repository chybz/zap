#include <unordered_map>

#include <zap/remote.hpp>
#include <zap/log.hpp>

namespace zap {

const remote_to_string_map&
get_remote_to_string_map()
{
    static const remote_to_string_map m = {
        { remote_type::none, "none" },
        { remote_type::github, "github" },
        { remote_type::gitlab, "gitlab" },
        { remote_type::bitbucket, "bitbucket" }
    };

    return m;
}

const string_to_remote_map&
get_string_to_remote_map()
{
    static const string_to_remote_map m = {
        { "none", remote_type::none },
        { "github", remote_type::github },
        { "gitlab", remote_type::gitlab },
        { "bitbucket", remote_type::bitbucket }
    };

    return m;
}

const std::string&
to_string(remote_type t)
{ return get_remote_to_string_map().at(t); }

remote_type
from_string(const std::string& s)
{
    const auto& m = get_string_to_remote_map();

    auto it = m.find(s);

    die_if(it == m.end(), "invalid remote type: ", s);

    return it->second;
}

bool
valid_remote(const std::string& s)
{ return get_string_to_remote_map().contains(s); }

void
check_remote(const std::string& s)
{ die_unless(valid_remote(s), "invalid remote type: ", s); }

}
