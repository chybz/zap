#pragma once

#include <zap/layout.hpp>
#include <zap/types.hpp>

namespace zap::layouts {

class app : public zap::layout
{
public:
    app(const std::string& project_dir);

    app(
        const std::string& label,
        const std::string& project_dir,
        const zap::string_map& sub_dirs
    );

    virtual ~app();

    bool detect() const override;

    void find_targets(zap::project& p) override;

private:
    bool detect_libs() const;
    bool detect_self_contained(zap::target_type type) const;

    void find_libs(zap::project& p);

    void find_targets(
        zap::project& p,
        zap::target_type type,
        zap::targets& targets
    );

    zap::string_map sub_dirs_;
};

}
