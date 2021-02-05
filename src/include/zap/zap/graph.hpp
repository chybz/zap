#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <stack>

#include <zap/types.hpp>

namespace zap {

class graph
{
public:
    graph();
    graph(const strings& names);

    graph(const graph& other) = default;
    graph(graph&& other) = default;

    graph& operator=(const graph& other) = default;
    graph& operator=(graph&& other) = default;

    virtual ~graph();

    void add_node(const std::string& name, const std::string& from = {});
    void add_nodes(const strings& names, const std::string& from = {});
    void add_nodes(const string_set& names, const std::string& from = {});

    void add_edge(const std::string& from, const std::string& to);

    void build();
    void clear();

    // From least to most dependent
    const strings& ordered() const;

    // From most to least dependent
    const strings& reversed() const;

    bool is_tree() const;

private:
    struct node
    {
        std::string name;
        int index;
        int low_link;
        bool on_stack;
        string_set edges;
        std::size_t dep_count = 0;

        void reset()
        {
            index = -1;
            low_link = -1;
            on_stack = false;
        }
    };

    using node_ref = std::reference_wrapper<node>;
    using node_map = std::unordered_map<std::string, node>;
    using node_stack = std::vector<node_ref>;

    void reset();

    void order_func(
        node& v,
        strings& ordered,
        node_stack& st,
        int& index
    );

    node_map nodes_;
    strings ordered_;
    strings reversed_;
};

}
