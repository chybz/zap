#pragma once

#include <string>

namespace zap {

class os_info
{
public:
    os_info();
    virtual ~os_info();

    const std::string& sys_name() const;
    const std::string& dist_name() const;
    const std::string& arch() const;
    const std::string& host_arch() const;

    bool is_unix() const;
    bool is_linux() const;
    bool is_debian() const;
    bool is_redhat() const;
    bool is_freebsd() const;
    bool is_netbsd() const;
    bool is_openbsd() const;
    bool is_osx() const;
    bool is_windows() const;

private:
    std::string sys_name_;
    std::string dist_name_;
    std::string arch_;
    std::string host_arch_;

    bool is_unix_ = false;
    bool is_linux_ = false;
    bool is_debian_ = false;
    bool is_redhat_ = false;
    bool is_freebsd_ = false;
    bool is_netbsd_ = false;
    bool is_openbsd_ = false;
    bool is_osx_ = false;
    bool is_windows_ = false;
};

}
