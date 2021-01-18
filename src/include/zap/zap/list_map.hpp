#pragma once

#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <unordered_map>

namespace zap {

template <
    typename T,
    typename K = std::string
>
struct list_map
{
    using value_type = T;
    using key_type = K;
    using reference = value_type&;
    using const_reference = const value_type&;

    using value_list = std::vector<value_type>;
    using key_list = std::vector<key_type>;
    using map = std::unordered_map<key_type, std::size_t>;
    using iterator = typename value_list::iterator;
    using const_iterator = typename value_list::const_iterator;

    iterator begin()
    { return l.begin(); }

    iterator end()
    { return l.end(); }

    const_iterator begin() const
    { return l.begin(); }

    const_iterator end() const
    { return l.end(); }

    reference front()
    { return l.front(); }

    const_reference front() const
    { return l.front(); }

    reference back()
    { return l.back(); }

    const_reference back() const
    { return l.back(); }

    std::size_t size() const
    { return l.size(); }

    bool empty() const
    { return l.empty(); }

    void clear()
    { l.clear(); keys_.clear(); m.clear(); }

    const value_list& values() const
    { return l; }

    const key_list& keys() const
    { return keys_; }

    const key_type& key(std::size_t pos) const
    { return keys_[pos]; }

    reference operator[](std::size_t pos)
    { return l[pos]; }

    const_reference operator[](std::size_t pos) const
    { return l[pos]; }

    reference operator[](const key_type& k)
    { return l[m.find(k)->second]; }

    const_reference operator[](const key_type& k) const
    { return l[m.find(k)->second]; }

    std::size_t pos(const key_type& k) const
    { return m.find(k)->second; }

    bool has(const key_type& k) const
    { return m.find(k) != m.end(); }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void add(const key_type& k)
    { add(k, k); }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void add(key_type&& k)
    { add(k, std::forward<key_type>(k)); }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void add(const key_list& kl)
    {
        for (const auto& k : kl) {
            add(k, k);
        }
    }

    template <
        typename Enabled = std::enable_if_t<
            std::is_same_v<key_type, value_type>
        >
    >
    void add(key_list&& kl)
    {
        for (auto&& k : kl) {
            add(k, std::forward<key_type>(k));
        }
    }

    void add(const key_type& k, value_type&& t)
    {
        if (has(k)) {
            return;
        }

        keys_.push_back(k);
        m[k] = l.size();
        l.push_back(std::move(t));
    }

    void add(const key_type& k, const value_type& t)
    {
        if (has(k)) {
            return;
        }

        keys_.push_back(k);
        m[k] = l.size();
        l.push_back(t);
    }

    void upsert(const key_type& k, const value_type& t)
    {
        if (has(k)) {
            l[m[k]] = t;
        } else {
            add(k, t);
        }
    }

    void remove(const key_type& k)
    {
        if (!has(k)) {
            return;
        }

        std::size_t pos = m[k];

        l.erase(l.begin() + pos);
        keys_.erase(keys_.begin() + pos);
        m.erase(k);

        for (pos = 0; pos < keys_.size(); pos++) {
            m[keys_[pos]] = pos;
        }
    }

    void merge(const list_map& other)
    {
        for (const auto& k : other.keys_) {
            add(k, other[k]);
        }
    }

    template <typename Pred>
    void sort(Pred p)
    {
        std::vector<std::size_t> indices(l.size());

        std::iota(
            indices.begin(), indices.end(),
            static_cast<size_t>(0)
        );

        std::vector<bool> swapped(l.size(), false);

        std::sort(
            indices.begin(), indices.end(),
            [this,p](std::size_t left, std::size_t right) {
                return p(
                    l[left],
                    l[right]
                );
            }
        );

        std::size_t orig = 0;

        for (const auto i : indices) {
            if (i != orig && !swapped[i]) {
                std::swap(l[orig], l[i]);
                std::swap(keys_[orig], keys_[i]);
                m[keys_[orig]] = orig;
                m[keys_[i]] = i;
                swapped[orig] = true;
                swapped[i] = true;
            }

            orig++;
        }
    }

    value_list l;
    key_list keys_;
    map m;
};

}
