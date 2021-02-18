#include <iostream>

#include <nlohmann/json.hpp>

#include <zap/cmake/trace_parser.hpp>
#include <zap/utils.hpp>

namespace zap::cmake {

using json = nlohmann::json;

trace_parser::trace_parser(const std::string& file)
: file_(file)
{
    parse();
}

trace_parser::~trace_parser()
{}

void
trace_parser::parse()
{
    std::ifstream ifs(file_);

    for (std::string line; std::getline(ifs, line); ) {
        if (zap::contains(line, "\"cmd\":\"add_library\"")) {
            parse_library(line);
        } else if (zap::contains(line, "\"cmd\":\"target_include_directories\"")) {
            parse_library_includes(line);
        }
    }
}

void
trace_parser::parse_library(const std::string& line)
{
    auto l = json::parse(line);
}

void
trace_parser::parse_library_includes(const std::string& line)
{

}

}
