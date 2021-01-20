#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>
#include <unordered_map>

#include <zap/smart_ref.hpp>
#include <zap/vector_map_iterator.hpp>

namespace zap {

namespace detail {

template <typename, typename = std::void_t<>>
struct type_has_name : std::false_type {};

template <typename T>
struct type_has_name<
    T,
    std::void_t<decltype(std::declval<T>().name())>
> : std::true_type {};

template <typename T>
using type_has_name_t = typename type_has_name<T>::type;

template <typename T>
constexpr bool type_has_name_v = type_has_name<T>::value;

}

template <typename T, typename K>
struct vector_map_extract_name
{
    auto operator()(const T& v) const
    {
        using type = typename smart_ref<T>::type;

        if constexpr (detail::type_has_name_v<type>) {
            return smart_ref<T>::get(v).name();
        } else if (std::is_same_v<type, K>) {
            return K(smart_ref<T>::get(v));
        } else {
            static_assert(
                std::is_convertible_v<type, K>,
                "cannot convert value to key"
            );
        }
    }
};

template <
    typename T,
    typename K = std::string,
    typename KeyExtract = vector_map_extract_name<T, K>
>
struct vector_map
{
    using this_type = vector_map<T>;
    using value_type = T;
    using key_type = K;

    using smart_ref_type = smart_ref<T>;
    using element_type = typename smart_ref<T>::type;

    using reference = element_type&;
    using const_reference = const element_type&;

    using value_list = std::vector<value_type>;
    using key_list = std::vector<key_type>;
    using map = std::unordered_map<key_type, std::size_t>;
    using iterator = vector_map_iterator<value_type, non_const_iterator_tag>;
    using const_iterator = vector_map_iterator<value_type, const_iterator_tag>;

    iterator begin()
    { return iterator(l.data()); }
    iterator end()
    { return iterator(l.data() + l.size()); }
    const_iterator begin() const
    { return const_iterator(l.data()); }
    const_iterator end() const
    { return const_iterator(l.data() + l.size()); }
    reference front()
    { return smart_ref_type::get(l.front()); }
    const_reference front() const
    { return smart_ref_type::get(l.front()); }
    reference back()
    { return smart_ref_type::get(l.back()); }
    const_reference back() const
    { return smart_ref_type::get(l.back()); }

    std::size_t size() const
    { return l.size(); }

    bool empty() const
    { return l.empty(); }

    void clear()
    { l.clear(); keys_.clear(); m.clear(); }

    const key_list& keys() const
    { return keys_; }

    const key_type& key(std::size_t pos) const
    { return keys_[pos]; }

    value_list& ptrs()
    {
        static_assert(
            smart_ref_type::value,
            "not a smart pointer vector_map"
        );

        return l;
    }

    const value_list& ptrs() const
    {
        static_assert(
            smart_ref_type::value,
            "not a smart pointer vector_map"
        );

        return l;
    }

    value_type& ptr(std::size_t pos)
    {
        static_assert(
            smart_ref_type::value,
            "not a smart pointer vector_map"
        );

        return l[pos];
    }

    const value_type& ptr(std::size_t pos) const
    {
        static_assert(
            smart_ref_type::value,
            "not a smart pointer vector_map"
        );

        return l[pos];
    }

    value_type& ptr(const key_type& k)
    {
        static_assert(
            smart_ref_type::value,
            "not a smart pointer vector_map"
        );

        return l[m.find(k)->second];
    }

    const value_type& ptr(const key_type& k) const
    {
        static_assert(
            smart_ref_type::value,
            "not a shared_ptr vector_map"
        );

        return l[m.find(k)->second];
    }

    reference operator[](std::size_t pos)
    { return smart_ref_type::get(l[pos]); }

    const_reference operator[](std::size_t pos) const
    { return smart_ref_type::get(l[pos]); }

    reference operator[](const key_type& k)
    { return smart_ref_type::get(l[m.find(k)->second]); }

    const_reference operator[](const key_type& k) const
    { return smart_ref_type::get(l[m.find(k)->second]); }

    std::size_t pos(const key_type& k) const
    { return m.find(k)->second; }

    bool contains(const key_type& k) const
    { return m.find(k) != m.end(); }

    bool contains(const key_list& ks) const
    {
        std::size_t count = 0;

        for (const auto& k : ks) {
            count += contains(k);
        }

        return count == ks.size();
    }

    void rename(const key_type& from, const key_type& to)
    {
        std::size_t pos = m[from];

        keys_[pos] = to;
        m.erase(from);
        m[to] = pos;
    }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void push_back(const key_type& k)
    { push_back(k, k); }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void push_back(key_type&& k)
    { push_back(k, std::forward<key_type>(k)); }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void push_back(const key_list& kl)
    {
        for (const auto& k : kl) {
            push_back(k, k);
        }
    }

    void push_back(value_type&& t)
    { push_core(std::move(key_of_(t)), std::move(t)); }

    void push_back(const value_type& t)
    { push_core(std::move(key_of_(t)), t); }

    void push_back(const key_type& k, value_type& v)
    { push_core(k, std::move(v)); }

    void push_back(const key_type& k, const value_type& v)
    { push_core(k, v); }

    template <typename U, typename V>
    void push_core(U&& u, V&& v)
    {
        l.push_back(std::move(v));
        m[u] = l.size() - 1;
        keys_.push_back(std::move(u));
    }

    std::string
    to_string() const
    {
        std::ostringstream oss;
        oss << "vector_map: " << join(", ", keys_);
        return oss.str();
    }

    void dump() const
    { std::cout << to_string() << std::endl; }

    value_list l;
    key_list keys_;
    map m;
    KeyExtract key_of_;
};

}
