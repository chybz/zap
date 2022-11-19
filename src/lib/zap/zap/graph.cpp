#include <algorithm>

#include <zap/graph.hpp>

namespace zap {

graph::graph()
{}

graph::graph(const strings& nodes)
{ add_nodes(nodes); }

graph::~graph()
{}

void
graph::add_node(const std::string& name, const std::string& from)
{
    if (!nodes_.contains(name)) {
        nodes_[name] = node{ name };
    }

    if (!from.empty()) {
        add_edge(from, name);
    }
}

void
graph::add_nodes(const strings& names, const std::string& from)
{
    for (const auto& n : names) {
        add_node(n, from);
    }
}

void
graph::add_nodes(const string_set& names, const std::string& from)
{
    for (const auto& n : names) {
        add_node(n, from);
    }
}

void
graph::add_edge(const std::string& from, const std::string& to)
{
    nodes_.at(from).edges.insert(to);
    nodes_.at(to).dep_count++;
}

void
graph::build()
{
    // Please see:
    //
    // Tarjan's strongly connected components algorithm
    //
    // https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
    reversed_.clear();
    ordered_.clear();

    node_stack st;
    int index = 0;

    reset();

    for (auto& vp : nodes_) {
        auto& v = vp.second;

        if (v.index == -1) {
            order_func(v, reversed_, st, index);
        }
    }

    ordered_.resize(reversed_.size());

    std::reverse_copy(
        std::begin(reversed_),
        std::end(reversed_),
        std::begin(ordered_)
    );
}

void
graph::clear()
{ nodes_.clear(); }

const strings&
graph::ordered() const
{ return ordered_; }

const strings&
graph::reversed() const
{ return reversed_; }

bool
graph::is_tree() const
{
    std::size_t count = 0;

    for (const auto& p : nodes_) {
        count += p.second.dep_count == 0;
    }

    return count == 1;
}

void
graph::reset()
{
    for (auto& p : nodes_) {
        p.second.reset();
    }
}

void
graph::order_func(
    node& v,
    strings& ordered,
    node_stack& st,
    int& index
)
{
    // Set the depth index for v to the smallest unused index
    v.index = index;
    v.low_link = index;
    ++index;

    st.push_back(v);
    v.on_stack = true;

    // Consider successors of v
    for (const auto& wn : v.edges) {
        auto& w = nodes_.at(wn);

        if (w.index == -1) {
            // Successor w has not yet been visited; recurse on it
            order_func(w, ordered, st, index);
            v.low_link = std::min(v.low_link, w.low_link);
        } else if (w.on_stack) {
            // Successor w is in stack S and hence in the current SCC
            // If w is not on stack, then (v, w) is a cross-edge
            // in the DFS tree and must be ignored
            // Note: The next line may look odd - but is correct.
            // It says w.index not w.low_link; that is deliberate and
            // from the original paper
            v.low_link = std::min(v.low_link, w.index);
        }
    }

    // If v is a root node, pop the stack and generate an SCC
    if (v.low_link == v.index) {
        // start a new strongly connected component
        strings scc;

        for ( ; ; ) {
            node& w = st.back(); st.pop_back();

            w.on_stack = false;
            scc.push_back(w.name);

            if (w.name == v.name) {
                break;
            }
        }

        ordered.insert(ordered.end(), scc.begin(), scc.end());
    }
}

}
