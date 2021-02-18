#pragma once

#include <string>

namespace zap::cmake {

class trace_parser
{
public:
    trace_parser(const std::string& file);
    virtual ~trace_parser();

private:
    void parse();
    void parse_library(const std::string& line);
    void parse_library_includes(const std::string& line);

    std::string file_;
};

}
