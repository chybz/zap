#pragma once

#include <fstream>
#include <string>

#include <zap/toolchain.hpp>

namespace zap::cmake {

class toolchain_file
{
public:
    static
    void write(
        const toolchain& tc,
        const std::string& file
    );

private:
    static
    void write(
        std::ofstream& ofs,
        const std::string var,
        const std::string val
    );
};

}
