/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_AT_HPP__)
#define TYPEUTIL_AT_HPP__
#include <type_traits>
#include <utility>
#include <iostream>

namespace typu {

template < typename ... Ts >
struct type_list {
};

template <size_t Idx, size_t I, typename... Ts>
struct at_type_impl;

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

	constexpr static type from_types_(type_list< Ts ... >&&) { return std::declval< type >(); };

	using from_types = decltype(from_types_(std::declval< type_list< Ts ... > >()));
};

template < size_t Idx, typename Ts >
struct at_types
{
	template < typename ... As > 
	constexpr static decltype(auto) from_ts(type_list< As... >&&)
	{
		return std::declval< typename at_type< Idx, As... >::type >();
	}

	using type = decltype(from_ts(std::declval< Ts >()));
};

template <size_t Idx, size_t I, typename... Ts>
struct to_types_impl;

template <size_t Idx, typename CAR, typename... CDR>
struct to_types_impl< Idx, Idx, CAR, CDR... > {
	template < typename... AccTs >
	constexpr static decltype(auto) to(std::tuple< AccTs ... >&& acc)
	{
		return std::forward< decltype(acc) >(acc);
	}
};

template <size_t Idx, size_t I, typename CAR, typename... CDR >
struct to_types_impl< Idx, I, CAR, CDR... > {
	template < typename... AccTs >
	constexpr static decltype(auto) to(std::tuple< AccTs ... >&& acc)
	{
		return to_types_impl< Idx, I + 1, CDR...>::template to(std::tuple_cat(acc, std::declval< std::tuple< CAR > >()));
	}
};

/* Ts ‚Ì‚¤‚¿æ“ª‚©‚ç Num ŒÂ‚ğæ‚èo‚·.
   type ‚Í std::tuple, types ‚Í‘f–p‚È type_list
 */
template <size_t Num, typename...Ts >
struct to_types {
	using type = decltype(to_types_impl< Num, 0, Ts... >::template to(std::make_tuple()));

	template < typename...AccTs >
	constexpr static type_list< AccTs ... > from_tuple(std::tuple< AccTs ... >&& acc) { return std::declval< type_list< AccTs ... > >(); };
	
	using types = decltype(from_tuple(std::declval< type >()));
};
}

#endif
