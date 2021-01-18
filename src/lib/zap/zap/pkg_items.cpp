#include <zap/pkg_items.hpp>

namespace zap {

using pkg_items_map = std::unordered_map<std::string, zap::strings>;

void
merge_items(pkg_items_map& to, pkg_items_map& from)
{
    for (auto&& p : from) {
        auto it = to.find(p.first);

        if (it != to.end()) {
            it->second.insert(
                it->second.end(),
                std::make_move_iterator(p.second.begin()),
                std::make_move_iterator(p.second.end())
            );
        } else {
            to.insert(std::move(p));
        }
    }
}

}
