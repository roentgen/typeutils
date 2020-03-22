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

	template < typename ...C > constexpr static auto concat(type_list< C... >&&) -> type_list< Ts..., C... >;
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

	/*type �� native �z��̂Ƃ� from_types_() ���z���Ԃ��֐��Ƃ��Ē�`�ł��Ȃ����ߔp�~ */
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

/* Ts �̂����擪���� Num �����o��.
   type �� std::tuple, types �͑f�p�� type_list

   std::tuple �� native array ���܂ނƃR���X�g���N�g�ł��Ȃ��Ȃ邽��, aggregate ����͓����I�� types �݂̂𗘗p����.
   type �͔p�~������.
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

template < typename T, typename F, size_t ...Idx >
constexpr std::array< typename std::result_of< F(typename at_types< 0, T >::type) >::type, sizeof...(Idx) >
map_apply_impl(F&& f, std::index_sequence< Idx... >&&)
{
	return {{f(typename at_types< Idx, T >::type{})...}};
}
	
template < typename F, typename ...Ts >
constexpr auto map_apply(type_list< Ts... >&& t, F&& fun)
{
	auto idx = std::make_index_sequence< sizeof...(Ts) >();
	return map_apply_impl< type_list<Ts...> >(std::forward<F>(fun), std::move(idx));
}

}


#endif
