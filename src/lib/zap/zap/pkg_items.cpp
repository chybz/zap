#include <zap/pkg_items.hpp>

namespace zap {

void
merge_pkg_items(pkg_items_map& to, pkg_items_map& from)
{
    for (auto it = from.begin(); it != from.end(); it = from.erase(it)) {
        auto& from_list = it->second;
        auto& to_list = to[it->first];

        to_list.insert(
            to_list.end(),
            std::make_move_iterator(from_list.begin()),
            std::make_move_iterator(from_list.end())
        );
    }

    from.clear();
}

}
