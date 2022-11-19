#pragma once

#include <string>

#include <zap/project.hpp>

namespace zap {

class layout
{
public:
    layout(
        const std::string& label,
        const std::string& project_dir
    );

    virtual ~layout();

    const std::string& label() const;

    virtual bool detect() const = 0;

    virtual void find_targets(project& p) = 0;

protected:
    std::string label_;
    std::string project_dir_;
    target_type_dirs dirs_;
};

layout&
get_layout(const std::string& dir);

}
