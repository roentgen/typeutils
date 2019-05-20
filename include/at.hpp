/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_AT_HPP__)
#define TYPEUTIL_AT_HPP__
#include <type_traits>
#include <utility>
#include <tuple>

namespace typu {

template < typename ... Ts >
struct type_list {
	/* deduce from arg  */
	template < template < typename ... > class Compo >
	static constexpr auto rewrap(type_list<Ts...>&&) -> Compo< Ts... >;

	template < template < typename ... > class Compo >
	using rewrap_t = Compo< Ts ... >;

	template < typename T >	using cons = type_list< Ts..., T >;
};

template <typename ...Ts> constexpr size_t type_list_size(type_list<Ts...>&&) { return sizeof...(Ts); }
	
template <size_t Idx, size_t I, typename... Ts>
struct at_type_impl { using type = void; };

template <size_t Idx, size_t I, typename CAR, typename... CDR>
struct at_type_impl< Idx, I, CAR, CDR... > {
    using type = typename at_type_impl< Idx, I + 1, CDR... >::type;
};

template <size_t Idx, typename CAR, typename... CDR>
struct at_type_impl< Idx, Idx, CAR, CDR... > {
    using type = CAR;
};

template <size_t Idx, typename... Ts>
struct at_type {
    using type = typename at_type_impl< Idx, 0, Ts... >::type;

	/*type が native 配列のとき from_types_() が配列を返す関数として定義できないため廃止 */
	//constexpr static auto from_types_(type_list< Ts ... >&&) -> type;
	//using from_types = decltype(from_types_(std::declval< type_list< Ts ... > >()));
};

template < size_t Idx, typename Ts >
struct at_types {
	static_assert(type_list_size(Ts{}) > Idx, "Idx out of range");
	
	template < typename ... As > 
	constexpr static auto from_ts(type_list< As... >&&)
		-> typename at_type< Idx, As... >::type;

	using type = decltype(from_ts(std::declval< Ts >()));
};

template < size_t Idx >
struct at_types< Idx, type_list<> > {
	using type = void;
};
	
template <size_t Idx, size_t I, typename... Ts>
struct to_types_impl;

template <size_t Idx, typename CAR, typename... CDR>
struct to_types_impl< Idx, Idx, CAR, CDR... > {
	template < typename... AccTs >
	constexpr static decltype(auto) to(type_list< AccTs ... >&& acc)
	{
		return std::forward< decltype(acc) >(acc);
	}
};

template <size_t Idx, size_t I, typename CAR, typename... CDR >
struct to_types_impl< Idx, I, CAR, CDR... > {
	template < typename... AccTs >
	constexpr static decltype(auto) to(type_list< AccTs ... >&&)
	{
		return to_types_impl< Idx, I + 1, CDR...>::template to(type_list< AccTs..., CAR >{});
	}
};

/* Ts のうち先頭から Num 個を取り出す.
   type は std::tuple, types は素朴な type_list

   std::tuple が native array を含むとコンストラクトできなくなるため, aggregate からは内部的に types のみを利用する.
   type は廃止したい.
 */
template <size_t Num, typename...Ts >
struct to_types {
	using types = decltype(to_types_impl< Num, 0, Ts... >::template to(std::declval< type_list<> >()));
	template < typename...AccTs >
	constexpr static auto from_ts(type_list< AccTs ... >&& acc) -> std::tuple< AccTs... >;
	using type = decltype(from_ts(std::declval< types >()));
};

/*
template <size_t Num, typename...Ts >
struct to_type {
	template < typename...AccTs >
	constexpr static std::tuple< AccTs ... > from_ts(type_list< AccTs ... >&& acc) -> std::tuple< AccTs... >;
	

};
*/
}

#endif
