#ifdef __unix__
#include <sys/utsname.h>
#else
#endif

#include <zap/os_info.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/prog.hpp>

namespace zap {

os_info::os_info()
{
#ifdef __unix__
    is_unix_ = true;

    utsname un;

    die_unless(
        ::uname(&un) == 0,
        "failed to retrieve kernel information"
    );

    sys_name_.assign(un.sysname);
    arch_.assign(un.machine);

    if (sys_name_ == "Linux") {
        is_linux_ = true;

        auto id = get_line("lsb_release", { .args = { "-is" } });

        if (id == "Debian" || id == "Ubuntu") {
            is_debian_ = true;

            host_arch_ = get_line(
                "dpkg-architecture",
                { .args = { "-qDEB_HOST_MULTIARCH" } }
            );

        } else {
            die("distribution '", id, "' is not yet supported");
        }
    }

    // TODO: implement the rest
#else
#endif
}

os_info::~os_info()
{}

const std::string&
os_info::sys_name() const
{ return sys_name_; }

const std::string&
os_info::dist_name() const
{ return dist_name_; }

const std::string&
os_info::arch() const
{ return arch_; }

const std::string&
os_info::host_arch() const
{ return host_arch_; }

bool
os_info::is_unix() const
{ return is_unix_; }

bool
os_info::is_linux() const
{ return is_linux_; }

bool
os_info::is_debian() const
{ return is_debian_; }

bool
os_info::is_redhat() const
{ return is_redhat_; }

bool
os_info::is_freebsd() const
{ return is_freebsd_; }

bool
os_info::is_netbsd() const
{ return is_netbsd_; }

bool
os_info::is_openbsd() const
{ return is_openbsd_; }

bool
os_info::is_osx() const
{ return is_osx_; }

bool
os_info::is_windows() const
{ return is_windows_; }


}
