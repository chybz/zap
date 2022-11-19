#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

#include <re2/re2.h>

#include <zap/types.hpp>

namespace zap {

struct module_match_info
{
    bool matched = false;
    std::string module;
    std::string component;
    std::string config;
};

using header_match_cb = std::function<
    void(
        const std::string_view&,
        const string_set_map& config_targets,
        module_match_info& info
    )
>;

struct header_to_module
{
    re2::RE2 re;
    header_match_cb cb;

    bool match(
        const std::string& header,
        const string_set_map& config_targets,
        module_match_info& info
    ) const;
};

using header_to_module_ptr = std::unique_ptr<header_to_module>;
using header_to_module_ptrs = std::vector<header_to_module_ptr>;

struct module
{
    std::string name;
    header_to_module_ptrs detectors;

    void add(const std::string& pat, header_match_cb cb);

    bool match(
        const std::string& header,
        const string_set_map& config_targets,
        module_match_info& info
    ) const;
};

using module_map = std::unordered_map<std::string, module>;

class frameworks
{
public:
    frameworks();
    virtual ~frameworks();

    module_match_info match(
        const std::string& header,
        const string_set_map& config_targets
    ) const;

private:
    module& add_module(const std::string& name, const std::string& prefix);

    module_map modules_;
};

}
