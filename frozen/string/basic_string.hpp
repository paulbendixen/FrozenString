#if !defined FROZEN_STRING_BASIC_STRING_HPP_INCLUDED
#define      FROZEN_STRING_BASIC_STRING_HPP_INCLUDED

#include <cstddef>
#include <stdexcept>
#include <ostream>
#include <string>

#include "./detail/macros.hpp"
#include "./detail/digits.hpp"
#include "./detail/util.hpp"
#include "./detail/indices.hpp"
#include "./detail/array_wrapper.hpp"
#include "./detail/strlen.hpp"
#include "../type_traits_aliases.hpp"
#include "./to_string.hpp"

namespace frozen {

using std::size_t;

// forward declaration
template<class Char, size_t N>
class basic_string;


template<class Char, size_t M, size_t N>
inline constexpr bool operator==(Char const (&lhs)[M], basic_string<Char, N> const& rhs)
{
    return rhs == lhs;
}

template<class Char, size_t M, size_t N>
inline constexpr bool operator!=(Char const (&lhs)[M], basic_string<Char, N> const& rhs)
{
    return rhs != lhs;
}

namespace detail {

    template<class Char, size_t M, size_t N>
    class operator_plus_impl{
        size_t const size_lhs;
        size_t const size_rhs;

    public:
        constexpr operator_plus_impl(size_t sl, size_t sr)
            : size_lhs(sl), size_rhs(sr) {}

        template<class ArrayL, class ArrayR, size_t... IndicesL, size_t... IndicesR>
        constexpr basic_string<Char, M+N> operator()(ArrayL const& lhs, ArrayR const& rhs, detail::indices<IndicesL...>, detail::indices<IndicesR...>) const
        {
            return {{{
                        ( IndicesL < size_lhs ? lhs[IndicesL] :
                          IndicesL < size_lhs + size_rhs ? rhs[IndicesL - size_lhs] :
                          '\0'
                        )...,
                        ( IndicesR + (M - size_lhs ) < size_rhs ?
                            rhs[IndicesR + (M - size_lhs)] :
                            '\0'
                        )...
                   }}};
        }
    };

} // namespace detail
template<class Char, size_t M, size_t N, size_t rlen = basic_string<Char, N>::len>
inline constexpr basic_string<Char, M+rlen> operator+(Char const (&lhs)[M], basic_string<Char, N> const& rhs)
{
    return detail::operator_plus_impl<Char, M, rlen>(detail::strlen(lhs), detail::strlen(rhs))(lhs, rhs, detail::make_indices<0, M>(), detail::make_indices<0, rlen>());
}

namespace detail {

    template<class Char, size_t N, size_t... IndicesR>
    inline constexpr basic_string<Char, N+1> operator_plus_char_impl(Char lhs, basic_string<Char, N> const& rhs, detail::indices<IndicesR...>)
    {
        return {{{lhs, rhs[IndicesR]...}}};
    }

} // namespace detail

template<class Char, size_t N,
         size_t rlen = basic_string<Char, N>::len,
         class = alias::enable_if<detail::check_char<Char>::value>>
inline constexpr basic_string<Char, rlen+1> operator+(Char lhs, basic_string<Char, N> const& rhs)
{
    return detail::operator_plus_char_impl(lhs, rhs, detail::make_indices<0, rlen>());
}

template<class Num, class Char, size_t N,
         class = alias::enable_if<
                     !detail::check_char<Num>::value &&
                     ( std::is_floating_point<alias::decay<Num>>::value ||
                       std::is_integral<alias::decay<Num>>::value )
                 >>
inline constexpr auto operator+(Num lhs, basic_string<Char, N> const& rhs)
    -> decltype(to_basic_string<Char>(std::declval<Num>()) + rhs)
{
    return to_basic_string<Char>(lhs) + rhs;
}

template<class Char, size_t N, size_t M>
inline constexpr bool operator<(Char const (&lhs)[N], basic_string<Char, M> const& rhs)
{
    return rhs > lhs;
}

template<class Char, size_t N, size_t M>
inline constexpr bool operator>(Char const (&lhs)[N], basic_string<Char, M> const& rhs)
{
    return rhs < lhs;
}

template<class Char, size_t N, size_t M>
inline constexpr bool operator<=(Char const (&lhs)[N], basic_string<Char, M> const& rhs)
{
    return rhs >= lhs;
}

template<class Char, size_t N, size_t M>
inline constexpr bool operator>=(Char const (&lhs)[N], basic_string<Char, M> const& rhs)
{
    return rhs <= lhs;
}

template<size_t N>
inline std::ostream &operator<<(std::ostream &os, basic_string<char, N> const& rhs)
{
    os << rhs.data();
    return os;
}

template<class C, size_t N,
         class = alias::enable_if< !std::is_same<C, char>::value >
        >
inline std::wostream &operator<<(std::wostream &os, basic_string<C, N> const& rhs)
{
    os << rhs.data();
    return os;
}


template<class Char, size_t N>
class basic_string{
public:

    // type definitions
    typedef typename detail::check_char<Char>::type value_type;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef value_type& reference;
    typedef value_type const& const_reference;
    typedef size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef detail::make_indices<0, N> indices_type;
    typedef basic_string<Char, N> self_type;

    static constexpr size_type len = N ? N : 1;

    // ctor definitions
    basic_string() = default;

    template<size_t M, class = alias::enable_if<M <= len>>
    constexpr basic_string(Char const (&str)[M])
        : basic_string(str, detail::make_indices<0, len>())
    {}

    constexpr basic_string(Char const *str)
        : basic_string(str, detail::strlen(str, len), detail::make_indices<0, len>())
    {}

    // implicit conversion from <braced initializer list> to array_wrapper is expected
    constexpr basic_string(detail::array_wrapper<Char, len> aw)
        : elems(aw)
    {}

    // implicit conversion from shorter string
    template<size_t M>
    constexpr basic_string(basic_string<Char, M> const& lhs)
        : basic_string(lhs, detail::make_indices<0, len>())
    {
        FROZEN_STATIC_ASSERT(N>=M);
    }

    // access
    constexpr value_type front() const noexcept
    {
        return elems.data[0];
    }

    constexpr value_type back() const noexcept
    {
        return elems.data[len-1];
    }

    constexpr const_pointer data() const noexcept
    {
        return elems.data;
    }

    constexpr value_type at(size_type idx) const
    {
        return idx < len ? elems.data[idx] : throw std::out_of_range("index out of range");
    }

    // iterator interfaces
    constexpr const_iterator begin() const noexcept
    {
        return elems.data;
    }

    constexpr const_iterator end() const noexcept
    {
        return elems.data + len;
    }

    iterator begin() noexcept
    {
        return elems.data;
    }

    iterator end() noexcept
    {
        return elems.data + len;
    }

    constexpr size_type max_size() const noexcept
    {
        return N;
    }

    constexpr bool empty() const noexcept
    {
        return front()=='\0';
    }

    constexpr size_type size() const noexcept
    {
        return size_impl(0);
    }

    // operators
    constexpr value_type operator[](size_type idx) const
    {
        return this->at(idx);
    }

    template<size_t M, size_t rlen = basic_string<Char, M>::len>
    constexpr basic_string<Char, len+rlen> operator+(basic_string<Char, M> const& rhs) const
    {
        return detail::operator_plus_impl<Char, len, rlen>(detail::strlen(elems, len), detail::strlen(rhs))(elems, rhs, detail::make_indices<0, len>(), detail::make_indices<0, rlen>());
    }

    template<size_t M>
    constexpr basic_string<Char, len+M> operator+(Char const (&rhs)[M]) const
    {
        return detail::operator_plus_impl<Char, len, M>(detail::strlen(elems), detail::strlen(rhs))(elems, rhs, detail::make_indices<0, len>(), detail::make_indices<0, M>());
    }

    template<class C, class = alias::enable_if<detail::check_char<C>::value>>
    constexpr basic_string<Char, N+1> operator+(C rhs) const
    {
        return operator_plus_char_impl(rhs, detail::strlen(elems), detail::make_indices<0, len>());
    }

    template<class Num,
             class = alias::enable_if<
                         !detail::check_char<alias::decay<Num>>::value &&
                         ( std::is_integral<alias::decay<Num>>::value ||
                           std::is_floating_point<alias::decay<Num>>::value )
                         >
            >
    constexpr auto operator+(Num n) const
        -> decltype(std::declval<self_type>() + to_basic_string<Char>(std::declval<Num>())) const
    {
        return operator+(to_basic_string<Char>(n));
    }

    template<size_t M, size_t rlen = basic_string<Char, M>::len>
    constexpr bool operator==(basic_string<Char, M> const& rhs) const
    {
        return detail::strlen(elems, len)!=detail::strlen(rhs, rlen) ?
            false : operator_equal_impl(rhs, 0, detail::strlen(elems, len<rlen ? len : rlen));
    }

    template<size_t M>
    constexpr bool operator==(Char const (&rhs)[M]) const
    {
        return detail::strlen(elems, len)!=detail::strlen(rhs, M) ?
            false : operator_equal_impl(rhs, 0, detail::strlen(elems, len<M ? len : M));
    }

    template<size_t M>
    constexpr bool operator!=(basic_string<Char, M> const& rhs) const
    {
        return ! operator==(rhs);
    }

    template<size_t M>
    constexpr bool operator!=(Char const (&rhs)[M]) const
    {
        return ! operator==(rhs);
    }

    template<size_t M, size_t rlen = basic_string<Char, M>::len>
    constexpr bool operator<(basic_string<Char, M> const& rhs) const
    {
        return operator_less_impl(detail::strlen(elems, len), detail::strlen(rhs, rlen))(elems, rhs, 0);
    }

    template<size_t M>
    constexpr bool operator<(Char const(&rhs)[M]) const
    {
        return operator_less_impl(detail::strlen(elems, len), detail::strlen(rhs, M))(elems, rhs, 0);
    }

    template<size_t M>
    constexpr bool operator>=(basic_string<Char, M> const& rhs) const
    {
        return ! operator<(rhs);
    }

    template<size_t M>
    constexpr bool operator>=(Char const(&rhs)[M]) const
    {
        return ! operator<(rhs);
    }

    template<size_t M, size_t rlen = basic_string<Char, M>::len>
    constexpr bool operator>(basic_string<Char, M> const& rhs) const
    {
        return operator_greater_impl(detail::strlen(elems, len), detail::strlen(rhs, rlen))(elems, rhs, 0);
    }

    template<size_t M>
    constexpr bool operator>(Char const(&rhs)[M]) const
    {
        return operator_greater_impl(detail::strlen(elems, len), detail::strlen(rhs, M))(elems, rhs, 0);
    }

    template<size_t M>
    constexpr bool operator<=(basic_string<Char, M> const& rhs) const
    {
        return ! operator>(rhs);
    }

    template<size_t M>
    constexpr bool operator<=(Char const(&rhs)[M]) const
    {
        return ! operator>(rhs);
    }

    template<class C, size_t J>
    friend inline std::ostream &operator<<(std::ostream &os, basic_string<C, J> const& rhs);

    // conversions
    std::basic_string<Char> to_std_string() const
    {
        return {data()};
    }

private:
    template<size_t M, size_t... Indices>
    constexpr basic_string(Char const (&str)[M], detail::indices<Indices...>)
        : elems({{str[Indices]...}})
    {}

    template<size_t... Indices>
    constexpr basic_string(Char const *str, size_t len, detail::indices<Indices...>)
        : elems({{(Indices < len ? str[Indices] : static_cast<Char>('\0'))...}})
    {}

    template<size_t M, size_t... Indices>
    constexpr basic_string(basic_string<Char, M> const& rhs, detail::indices<Indices...>)
        : elems({{(Indices < basic_string<Char, M>::len ? rhs[Indices] : static_cast<Char>('\0'))...}})
    {}

    template<size_t... IndicesL>
    constexpr basic_string<Char, N+1> operator_plus_char_impl(Char rhs, size_t len_lhs, detail::indices<IndicesL...>) const
    {
        // use of explicit ctor for basic_string(Char)
        return {{{
                      ( IndicesL < len_lhs ? elems[IndicesL] :
                        IndicesL == len_lhs ? rhs : static_cast<Char>('\0') )...
               }}};
    }

    template<class Array>
    constexpr bool operator_equal_impl(Array const& rhs, size_t idx, size_t max_size) const
    {
        return idx == max_size ? true : elems[idx] == rhs[idx] && operator_equal_impl(rhs, idx+1, max_size);
    }

    class operator_less_impl{
        size_t const size_lhs;
        size_t const size_rhs;

    public:
        constexpr operator_less_impl(size_t sl, size_t sr)
            : size_lhs(sl), size_rhs(sr) {}

        template<class ArrayL, class ArrayR>
        constexpr bool operator()(ArrayL const& lhs, ArrayR const& rhs, size_t idx) const
        {
            return idx == size_rhs ? false :
                   idx == size_lhs ? true :
                   lhs[idx] < rhs[idx] ? true :
                   lhs[idx] > rhs[idx] ? false :
                                         operator()(lhs, rhs, idx+1);
        }
    };

    class operator_greater_impl{
        size_t const size_lhs;
        size_t const size_rhs;

    public:
        constexpr operator_greater_impl(size_t sl, size_t sr)
            : size_lhs(sl), size_rhs(sr) {}

        template<class ArrayL, class ArrayR>
        constexpr bool operator()(ArrayL const& lhs, ArrayR const& rhs, size_t idx) const
        {
            return idx == size_lhs ? false :
                   idx == size_rhs ? true :
                   lhs[idx] < rhs[idx] ? false :
                   lhs[idx] > rhs[idx] ? true :
                                         operator()(lhs, rhs, idx+1);
        }
    };

    constexpr size_type size_impl(size_type idx) const
    {
        return elems[idx] == '\0' || !(idx < len) ?
            0 : 1 + size_impl(idx+1);
    }

private:
    detail::array_wrapper<Char, len> const elems;
}; // class basic_string

} // namespace frozen


#endif    // FROZEN_STRING_BASIC_STRING_HPP_INCLUDED
