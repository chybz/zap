#include <algorithm>

#include <zap/executor.hpp>

namespace zap {

std::size_t
adjust_par_level(const executor& exec, std::size_t par_level)
{
    return
        par_level == 0
        ? exec.num_workers()
        : par_level
        ;
}

}
