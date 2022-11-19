#pragma once

#include <string>

#include <zap/command.hpp>
#include <zap/types.hpp>

namespace zap::commands {

struct install_opts
{
    std::string url;
    std::string file;
    std::string directory;
    zap::strings args;
};

class install : public zap::command
{
public:
    install(const zap::env& e, const install_opts& opts);
    virtual ~install();

    void operator()() final;

private:
    void install_url(const std::string& url);
    void install_directory(const std::string& dir);
    void install_archive(const archive_info& ai);

    install_opts opts_;
};

}
