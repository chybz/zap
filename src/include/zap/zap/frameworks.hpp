#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

#include <re2/re2.h>

namespace zap {

struct lib_info
{
    bool matched = false;
    std::string component;
    std::string lib_name;
};

using header_match_cb = std::function<
    void(const std::string_view&, lib_info& li)
>;

struct header_to_lib
{
    re2::RE2 re;
    header_match_cb cb;

    void match(const std::string& header, lib_info& li) const;
};

using header_to_lib_ptr = std::unique_ptr<header_to_lib>;
using header_to_lib_ptrs = std::vector<header_to_lib_ptr>;

struct module_match_info
{
    bool matched = false;
    std::string module;
    lib_info info;
};

struct module
{
    std::string name;
    header_to_lib_ptrs detectors;

    void add(const std::string& pat, header_match_cb cb);

    lib_info match(const std::string& header) const;
};

using module_map = std::unordered_map<std::string, module>;

class frameworks
{
public:
    frameworks();
    virtual ~frameworks();

    module_match_info match(const std::string& header) const;

private:
    module& add_module(const std::string& name, const std::string& prefix);

    module_map modules_;
};

}
