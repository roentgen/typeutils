/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_STR_HPP__)
#define TYPEUTIL_STR_HPP__
#include <type_traits>
#include <utility>
#include <stdint.h>

namespace typu {

/* DJB hash: �Փ˂��m���Ă���̂Œ��� */
constexpr uint64_t operator ""_hash(const char* str, size_t l)
{
	uint64_t h = 5381;
	while (auto c = *str ++) h = (h << 5) + h + c; // h * 33 + c
	return h;
}

constexpr size_t length(const char* str) { return str[0] != 0 ? 1 + length(str + 1) : 0; }

#if defined(__GNUC__)
/* gcc/clang �ł� GNU extension �Ƃ��Ă��� literal operator ���g����. */
template < typename T, T... Cs >
constexpr decltype(auto) operator ""_decompo() { return std::integer_sequence< T, Cs... >{}; }

#define DECO(str) str ## _decompo

#else

/* �������̂�����@ */
template <typename F, size_t ...Idx>
constexpr decltype(auto) decompose(std::index_sequence<Idx...>, F&& f) { return std::forward< F >(f)(Idx...); }

#define DECO(str) decompose(std::make_index_sequence< length(str) >(), [](auto... idx) { return std::integer_sequence< char, ((str)[decltype(idx){}])... >{}; })
#endif
}

#endif
