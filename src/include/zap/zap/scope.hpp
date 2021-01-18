#pragma once

#include <functional>
#include <vector>
#include <string>

namespace zap {

struct scope
{
    using scope_cb = std::function<void()>;
    using scope_cbs = std::vector<scope_cb>;

    ~scope();

    void clear();

    void push(scope_cb cb);
    void push_rmfile(const std::string& file);
    void push_rmpath(const std::string& dir);

    void if_ok(scope_cb cb);
    void if_ok_rmfile(const std::string& file);
    void if_ok_rmpath(const std::string& dir);

    void if_not_ok(scope_cb cb);
    void if_not_ok_rmfile(const std::string& file);
    void if_not_ok_rmpath(const std::string& dir);

private:
    void run(scope_cbs& cbs);

    void push(scope_cbs& cbs, scope_cb cb);
    void push_rmfile(scope_cbs& cbs, const std::string& file);
    void push_rmpath(scope_cbs& cbs, const std::string& dir);

    bool cleared_ = false;
    scope_cbs cbs_;
    scope_cbs ok_cbs_;
    scope_cbs not_ok_cbs_;
};

}
