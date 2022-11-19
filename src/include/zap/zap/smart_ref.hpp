#pragma once

#include <type_traits>
#include <memory>

namespace zap {

template <typename T>
struct smart_ref_helper : std::false_type
{
    using type = T;

    static
    type& get(type& e)
    { return e; }

    static
    const type& get(const type& e)
    { return e; }
};

template <typename T>
struct smart_ref_helper<std::shared_ptr<T>> : std::true_type
{
    using type = typename std::remove_cv<T>::type;
    using ptr_type = std::shared_ptr<type>;

    static
    type& get(ptr_type& p)
    { return *p; }

    static
    const type& get(const ptr_type& p)
    { return *p; }
};

template <typename T>
struct smart_ref_helper<std::unique_ptr<T>> : std::true_type
{
    using type = typename std::remove_cv<T>::type;
    using ptr_type = std::unique_ptr<type>;

    static
    type& get(ptr_type& p)
    { return *p; }

    static
    const type& get(const ptr_type& p)
    { return *p; }
};

template <typename T>
using smart_ref = smart_ref_helper<
    std::remove_reference_t<
        std::remove_cv_t<T>
    >
>;

}
