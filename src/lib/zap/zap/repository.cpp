#include <unordered_map>

#include <zap/repository.hpp>
#include <zap/log.hpp>

namespace zap {

const repository_to_string_map&
get_repository_to_string_map()
{
    static const repository_to_string_map m = {
        { repository_type::none, "none" },
        { repository_type::github, "github" },
        { repository_type::gitlab, "gitlab" },
        { repository_type::bitbucket, "bitbucket" }
    };

    return m;
}

const string_to_repository_map&
get_string_to_repository_map()
{
    static const string_to_repository_map m = {
        { "none", repository_type::none },
        { "github", repository_type::github },
        { "gitlab", repository_type::gitlab },
        { "bitbucket", repository_type::bitbucket }
    };

    return m;
}

const std::string&
to_string(repository_type t)
{ return get_repository_to_string_map().at(t); }

repository_type
to_repository(const std::string& s)
{
    const auto& m = get_string_to_repository_map();

    auto it = m.find(s);

    die_if(it == m.end(), "invalid repository type: ", s);

    return it->second;
}

bool
valid_repository(const std::string& s)
{ return get_string_to_repository_map().contains(s); }

void
check_repository(const std::string& s)
{ die_unless(valid_repository(s), "invalid repository type: ", s); }

}
