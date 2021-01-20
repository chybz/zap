#pragma once

#include <iterator>
#include <type_traits>

#include <zap/smart_ref.hpp>

namespace zap {

struct non_const_iterator_tag {};
struct const_iterator_tag {};

template <
    typename T,
    typename ConstTag = non_const_iterator_tag
>
class vector_map_iterator
{
public:
    using value_type = T;
    using smart_ref_type = smart_ref<value_type>;
    using smart_value_type = typename smart_ref_type::type;

    using reference = std::conditional_t<
        std::is_same_v<ConstTag, const_iterator_tag>,
        const smart_value_type&,
        smart_value_type&
    >;

    using pointer = std::conditional_t<
        std::is_same_v<ConstTag, const_iterator_tag>,
        const T*,
        T*
    >;

    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;

    vector_map_iterator(pointer ptr)
    : ptr_(ptr)
    {}

    vector_map_iterator& operator++()
    {
        ++ptr_;
        return *this;
    }

    vector_map_iterator operator++(int junk)
    {
        vector_map_iterator prev = *this;
        ptr_++;
        return prev;
    }

    reference operator*()
    { return smart_ref_type::get(*ptr_); }

    pointer operator->()
    { return ptr_; }

    bool operator==(const vector_map_iterator& rhs) const
    { return ptr_ == rhs.ptr_; }

    bool operator!=(const vector_map_iterator& rhs) const
    { return ptr_ != rhs.ptr_; }

private:
    pointer ptr_;
};

}
