/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_STR_HPP__)
#define TYPEUTIL_STR_HPP__
#include <type_traits>
#include <utility>
#include <stdint.h>
#include <assert.h>

namespace typu {

enum charclass : int {
	CH_NULL = 0,
	CH_DIGIT,
	CH_ALPHA,
	CH_SYM,
	CH_COMMA,
	CH_WSPACE,
	CH_PARENTHESIS_O,
	CH_PARENTHESIS_C,
	CH_BRACKET_O,
	CH_BRACKET_C,
	CH_BRACE_O,
	CH_BRACE_C,
	CH_PRINTABLE,
	CH_LO_ASCII,
	CH_HI_ASCII,
};

template < typename C > constexpr bool is_symbolic(C c)
{
	if ((c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') || 
		c == '_') return true;
	return false;
}

template < typename C > constexpr charclass check_charclass(C c)
{
	if (c > '~') return CH_HI_ASCII;
	if (c >= '0' && c <= '9') return CH_DIGIT;
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) return CH_ALPHA;
	if (c == '_') return CH_SYM;
	if (c == ',') return CH_COMMA;
	if (c == '	' || c == ' ' || c == 0x0a || c == 0x0d) return CH_WSPACE;
	if (c == '\0') return CH_NULL;
	if (c < ' ') return CH_LO_ASCII;
	if (c == '(') return CH_PARENTHESIS_O;
	if (c == ')') return CH_PARENTHESIS_C;
	if (c == '[') return CH_BRACKET_O;
	if (c == ']') return CH_BRACKET_C;
	if (c == '{') return CH_BRACE_O;
	if (c == '}') return CH_BRACE_C;

	return CH_PRINTABLE; // unknown but printable-ascii
}

template < typename C >
struct chtype {
	bool sym_ ;
	C c_;
	constexpr chtype(C c) : sym_(is_symbolic(c)), c_(std::move(c)) {};
	/* constexpr */ chtype(const chtype< C >& c) : sym_(c.sym_), c_(c.c_) {};
	constexpr explicit operator bool () { return sym_; };
	constexpr operator uint64_t () { return c_; }
	constexpr charclass type() { return check_charclass(c_); };
};


/* DJB hash: è’ìÀÇ™ímÇÁÇÍÇƒÇ¢ÇÈÇÃÇ≈íçà” */
constexpr uint64_t operator ""_hash(const char* str, std::size_t l)
{
	uint64_t h = 5381;
	chtype< char > c('\0');
	while ((c = chtype< char >(*str ++))) {
		h = (h << 5) + h + c; // h * 33 + c
	}
	assert(c.type() == CH_NULL || (false && "only alphabets, digits and _ are acceptable for future compatibility"));
	return h;
}

constexpr std::size_t length(const char* str) { return str[0] != 0 ? 1 + length(str + 1) : 0; }

#if defined(__GNUC__)
/* gcc/clang Ç≈ÇÕ GNU extension Ç∆ÇµÇƒÇ±ÇÃ literal operator Ç™égÇ¶ÇÈ. */
template < typename T, T... Cs >
constexpr decltype(auto) operator ""_decompo() { return std::integer_sequence< T, Cs... >{}; }

#define DECO(str) str ## _decompo

#else

/* ÇÊÇËâ¬î¿ê´ÇÃÇ†ÇÈï˚ñ@ */
template <typename F, size_t ...Idx>
constexpr decltype(auto) decompose(std::index_sequence<Idx...>, F&& f) { return std::forward< F >(f)(Idx...); }

#define DECO(str) decompose(std::make_index_sequence< length(str) >(), [](auto... idx) { return std::integer_sequence< char, ((str)[decltype(idx){}])... >{}; })
#endif
}

#endif
