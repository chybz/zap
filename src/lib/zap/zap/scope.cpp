#include <zap/scope.hpp>
#include <zap/utils.hpp>

namespace zap {

scope::~scope()
{
    run(cbs_);

    if (cleared_) {
        run(ok_cbs_);
    } else {
        run(not_ok_cbs_);
    }
}

void
scope::clear()
{
    not_ok_cbs_.clear();
    cleared_ = true;
}

void
scope::push(scope_cb cb)
{ push(cbs_, cb); }

void
scope::push_rmfile(const std::string& file)
{ push_rmfile(cbs_, file); }

void
scope::push_rmpath(const std::string& dir)
{ push_rmpath(cbs_, dir); }

void
scope::if_ok(scope_cb cb)
{ push(ok_cbs_, cb); }

void
scope::if_ok_rmfile(const std::string& file)
{ push_rmfile(ok_cbs_, file); }

void
scope::if_ok_rmpath(const std::string& dir)
{ push_rmpath(ok_cbs_, dir); }

void
scope::if_not_ok(scope_cb cb)
{ push(not_ok_cbs_, cb); }

void
scope::if_not_ok_rmfile(const std::string& file)
{ push_rmfile(not_ok_cbs_, file); }

void
scope::if_not_ok_rmpath(const std::string& dir)
{ push_rmpath(not_ok_cbs_, dir); }

void
scope::run(scope_cbs& cbs)
{
    while (!cbs.empty()) {
        cbs.back()();
        cbs.pop_back();
    }
}

void
scope::push(scope_cbs& cbs, scope_cb cb)
{ cbs.emplace_back(cb); }

void
scope::push_rmfile(scope_cbs& cbs, const std::string& file)
{ push(cbs, [file](){ rmfile(file); }); }

void
scope::push_rmpath(scope_cbs& cbs, const std::string& dir)
{ push(cbs, [dir](){ rmpath(dir); }); }

}
