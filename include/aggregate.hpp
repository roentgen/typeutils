#if !defined(TYPEUTIL_AGGREGATE_HPP__)
#define TYPEUTIL_AGGREGATE_HPP__
#include <utility>
#include <algorithm>
#include <type_traits>
#include <at.hpp>

namespace typu {

extern std::nullptr_t enabler;

struct compo_t {
};

using alignment_t = size_t;

template < typename T >
struct has_trv {
	template < typename U > static auto check(U u) -> decltype(u.trv(), std::true_type{}) { }
	static std::false_type check(...);
public:
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template < alignment_t Align, typename ...Ts >
struct sigma_size {
	static const size_t value = 0;
};

/* trv を持つ */
template < typename T, alignment_t Align = 1 > constexpr auto sizeof_() -> decltype(T::trv()) { return T::trv(); }

/* trv を持たない */
template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_() -> std::enable_if_t< !has_trv<T>::value, size_t >
{
	return sizeof(T);
}

template < alignment_t Align, typename CAR, typename ...CDR >
struct sigma_size< Align, CAR, CDR... > {
	static const size_t value = sizeof_< CAR >() + sigma_size< Align, CDR... >::value;
};

template < alignment_t Align, typename ...Ts >
constexpr size_t sigma_type_list(type_list< Ts ... >&&)
{
	return sigma_size< Align, Ts... >::value;
}

/* offset 計算のサブ関数:
   型 T が offset<Rest...>() を持っていれば Rest... のパターンに従って探索し, なければ終端する. */
template < alignment_t Align, typename T, size_t...Pos >
constexpr auto offset_() -> decltype(T::template offset< Align, Pos... >::value)
{
	return T::template offset< Align, Pos... >::value;
}

template < typename T, size_t... Pos >
struct has_offset {
	template < typename U > static auto check(U u) -> decltype(U::template offset< 0, Pos ... >::value, std::true_type{}) { }
	static std::false_type check(...);
public:
	static bool const value = decltype(check(std::declval<T>()))::value;
};

/* offset を持たないので Pos... の探索は終了 */
template < alignment_t Align, typename T, size_t...Pos >
constexpr auto offset_() -> std::enable_if_t< !has_offset<T, Pos...>::value, size_t >
{
	return std::is_empty< T >::value ? T() : sizeof(T);
}
	
/* aggregation
 */
template < typename ...S >
struct agg_t : public compo_t {
	//template < alignment_t Align = 1 >
	constexpr static size_t trv()
	{
		return sigma_size< 1, S... >::value;
	}

	template < alignment_t Align, size_t... Rest > struct offset { static const size_t value = 0; };
	template < alignment_t Align, size_t Cur >
	struct offset< Align, Cur > { static const size_t value = sigma_type_list< Align >(typename to_types< Cur, S... >::types{}); };

	template < alignment_t Align, size_t Cur, size_t ... Rest >
	struct offset< Align, Cur, Rest...>  {
		static const size_t value = sigma_type_list< Align >(typename to_types< Cur, S... >::types{}) +
			offset_< Align, typename at_type< Cur, S... >::type, Rest... >();
	};

	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using T = typename at_type< Pos, S... >::type;
		using type = typename T::template get< Align, Rest ... >::type;
	};

};

/* C++14 で std::max は constexpr 版ができたはずだが, clang-3.5+libc++ でなぜかだめ */
template < typename CAR, typename... CDR >
constexpr CAR constexpr_max(CAR&& car, CDR&&... cdr){
	CAR r = car;
	using s = std::initializer_list<int>;
	(void)s{ (void(r = r < cdr ? cdr : r),0)... };
	return r;
}


/* selector */
template < typename ...S >
struct sel_t : public compo_t {
	constexpr static size_t trv()
	{
		return constexpr_max(sizeof_<S>()...);
	}

	template < alignment_t Align, size_t... Rest > struct offset { static const size_t value = 0; };
	template < alignment_t Align, size_t Pos >	struct offset< Align, Pos > { static const size_t value = 0; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct offset< Align, Pos, Rest...>  {
		static const size_t value = offset_< Align, typename at_type< Pos, S... >::type, Rest... >();
	};

	template < alignment_t Align, size_t... Rest > struct get;
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
	};
	
#if 0
	template < size_t Pos, size_t...Rest >
	constexpr static size_t trv(size_t&& acc)
	{
		/* selector の先頭までのオフセットが求められているのでなければ集積型の全てについて max をとる.
		   (union は全部の max をとらないと不正確になる) */
		size_t r = acc;
		if (Pos > 0) 
			r += constexpr_max(sizeof_<S>()...);
		return sizeof...(Rest) > 0 ? at_type< Pos, S... >::type::template trv(std::move(r)) : r;
	}
#endif
};

template < typename T, T ...S >
struct sel_t_t {
	constexpr static size_t trv()
	{
		return constexpr_max(S...);
	}
};

template < typename S, alignment_t Align, typename Enabled = void >
struct type_t {
};

template < typename S, alignment_t Align>
struct type_t < S, Align, std::enable_if_t< std::is_base_of< compo_t, S >::value > > {
	using sub = S;
	static const size_t align = Align;
	
	constexpr static size_t trv()
	{
		return sizeof_<sub>();
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset()
	{
		//std::integer_sequence< size_t, (rest)... > s;
		return sub::template offset< Align, Rest... >::value;
	}

	template < size_t Cur, size_t ...Rest >
	using type = typename sub::template get< Align, Rest... >::type;
};

/* type_t<...>::template type<...> と同じだが, template と書きたくないので */
template < typename T, size_t... Pos >
struct get {
	using type = typename T::template type< Pos... >;
	static const size_t size = sizeof_<type>();
	static const size_t offset = T::template offset<Pos...>();
};


}
#endif
