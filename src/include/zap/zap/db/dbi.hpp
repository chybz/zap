#pragma once

#include <mutex>

#include <sqlite_orm/sqlite_orm.h>

#include <zap/db/storage_base.hpp>
#include <zap/log.hpp>
#include <zap/scope.hpp>

namespace zap::db {

enum class op
{
    read,
    write
};

template <typename StorageSpec>
struct dbi : storage_base
{
    using db_type = decltype(StorageSpec::make(std::string{}));

    dbi(const std::string& file)
    : db_(StorageSpec::make(file))
    {}

    virtual ~dbi()
    {}

    static
    storage_ptr new_storage(const std::string& file)
    { return std::make_unique<dbi>(file); }

    static
    dbi& get(storage_ptr& p)
    { return static_cast<dbi&>(*p); }

    static
    db_type& get_db(storage_ptr& p)
    { return get(p).db_; }

    template <typename Record, typename Col, typename Value>
    Record ensure_exists(const std::string& label, Col&& col, Value&& v)
    {
        using namespace sqlite_orm;

        auto rows = db_.template get_all<Record>(where(c(col) == v));

        die_if(
            rows.empty(),
            label, " '", v, "' doesn't exist"
        );

        return rows.front();
    }

    template <typename Record, typename Col, typename Value>
    void ensure_not_exists(const std::string& label, Col&& col, Value&& v)
    {
        using namespace sqlite_orm;

        auto count = db_.template count<Record>(where(c(col) == v));

        die_if(
            count > 0,
            label, " '", v, "' already exists"
        );
    }

    template <typename Record, typename Callable>
    void with_record(Callable&& cb)
    {
        auto rows = db_.template get_all<Record>();

        Record r;
        Record* rptr = &r;

        if (!rows.empty()) {
            rptr = std::addressof(rows.front());
        }

        if (cb(*rptr)) {
            db_.replace(*rptr);
        }
    }

    template <typename Callable>
    void exec(Callable&& cb, op o)
    {
        switch (o) {
            case op::read:
            exec_read(std::forward<Callable>(cb));
            break;
            case op::write:
            exec_write(std::forward<Callable>(cb));
            break;
        }
    }

    template <typename Callable>
    void exec_write(Callable&& cb)
    {
        try {
            lock_type l(m_);

            auto guard = db_.transaction_guard();
            scope s;
            cb(s);
            s.clear();
            guard.commit();
        } catch (...) {
            throw;
        }
    }

    template <typename Callable>
    void exec_read(Callable&& cb)
    {
        lock_type l(m_);

        scope s;
        cb(s);
        s.clear();
    }

private:
    using mutex_type = std::recursive_mutex;
    using lock_type = std::lock_guard<mutex_type>;

    mutex_type m_;
    db_type db_;
};

}
