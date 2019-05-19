/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_LOOKUP_HPP__)
#define TYPEUTIL_LOOKUP_HPP__

#include "aggregate.hpp"
#include "at.hpp"
#include <type_traits>
#include <stdint.h>

namespace typu {

using symbol_t = uint64_t;

template < typename T, bool Comp >
struct named_impl ;

template <>
struct named_impl<void, false > {
	static constexpr size_t elementsof() { return 0; };
	static constexpr alignment_t alignof_() { return 0; };

	template < typename X >
	constexpr static auto rewrap(X&&, type_list<>&&) -> void;
};

template < typename T >
struct named_impl<T, false > {
	static constexpr size_t elementsof() { return 1; };
	static constexpr alignment_t alignof_() { return typu::alignof_<T>(); };

	template < typename X, typename Y >
	constexpr static auto rewrap(X&&, type_list<Y>&&) -> Y;

	template < typename X >
	constexpr static auto rewrap(X&&, type_list<>&&) -> void;
};

template < typename T >
struct named_impl<T, true > {
	template < alignment_t Align = 1 >
	constexpr static size_t trv(size_t place = 0) { return T::template trv<Align>(place); }
	template < alignment_t Align = 1 >
	static constexpr size_t placement(size_t place = 0) { return T::template place<Align>(place); }
	
	template < alignment_t Align, size_t Acc, size_t... Rest > struct offset { static const size_t value = T::template offset< Align, Acc, Rest...>::value; };

	template < alignment_t Align, size_t ... Rest >
	struct get { using type = typename T::template get< Align, Rest ...>::type; };

	static constexpr size_t elementsof() { return T::elementsof(); };
	static constexpr alignment_t alignof_() { return typu::alignof_<T>(); };
	static constexpr size_t countin() { return T::countin(); }

	/* template <typename ...> typename Compo �� Compo �𐄘_�x�[�X�� rewrap ������������, 
	   �ł��Ȃ��̂ŃI�[�o�[���[�h�Ō��肷��.
	*/
	template < typename ... Ts, typename ... Ss >
	constexpr static auto rewrap(agg_t<Ts...>&&, type_list<Ss...>&&) -> agg_t<Ss...>;

	template < typename ... Ts, typename ... Ss >
	constexpr static auto rewrap(sel_t<Ts...>&&, type_list<Ss...>&&) -> sel_t<Ss...>;

	template < typename S0, typename S1, alignment_t Align >
	constexpr static auto rewrap(type_t<S0, Align>&&, type_list<S1>&&) -> S1;
};

/* NOTE:
   named_t �� get interface ���T�|�[�g���邩��, ���e�^ T �̓����ɂ���Č��肷��.
   named_t ���g�� get ���T�|�[�g����Ƃ͌���Ȃ�.
   ����� get �� Pos... �� named_t �����������悤�ɂ��邽��, T �̎����Ƀf�B�X�p�b�`���邩��ł���. 
   T=agg_t< type, named_t< 0, X >, type > �̂Ƃ�, X �� T::get<1> �ŃA�N�Z�X�����ׂ���, ���O�t���ɂ���� T::get<1, 0> �Ƃ͂Ȃ�Ȃ�.
   ���̂���, get interface �̗L���ɂ���� named_t �����ڂ� C++ �̌^�ł���Ƃ�, fold/map �̑Ώۂɂ͂Ȃ�Ȃ��Ȃ�. (���e�^ T �� get interface ���T�|�[�g���Ȃ�����)
   agg_t< named_t<0, agg_t< X > > >::fold<...> �͓�����, agg_t< named_t<0, X > >::fold<...> �͓����Ȃ��Ƃ������ƂɂȂ��Ă��܂�.
   ���������邽��, named_t �� get �������Ȃ��ꍇ�ł� fold/mapped �𒼐ڎ�����, has_fold/has_mapped ���g���Č^�`�F�b�N���s���K�v��������.
 */
template < symbol_t N, typename T >
struct named_t : public named_impl< T, has_get<T>::value > {
	static const symbol_t name = N;
	using type = T;

	template < typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
	struct fold {
		static const Acc value = extract_S< Acc, Acc0, T >::template eval<Fun...>();
	};

	template < template < typename... > class ...M > struct mapped {
		template < template <typename> class F, typename ...Ts >
		static constexpr auto filter(type_list<Ts...>&&) -> typename filter_impl< F, type_list<>, Ts... >::type;
		
		/* named_t �͌^�̍\����͈Ӗ��̂Ȃ� notation �Ƃ��ĐU��������, map �ɂ���ď�������Ȃ��悤 rewrap ���� */
		using rawtypelist = type_list< typename apply_impl< typename map_raw_if_traversable< has_mapped<T>::value, T, M... >::type, M... >::type >;
		using newrawtype = decltype(named_impl< T, has_get<T>::value >::rewrap(std::declval<T>(), std::declval<rawtypelist>()));
		using rawtype = named_t< N, newrawtype >;
		
		using typelist = type_list< typename apply_impl< typename map_if_traversable< has_mapped<T>::value, T, M... >::type, M... >::type >;
		using filtered = decltype(filter< not_empty_composite >(filter< not_void >(rawtypelist{})));

		using newtype = decltype(named_impl< T, has_get<T>::value >::rewrap(std::declval<T>(), std::declval<filtered>()));
		using type = typename std::conditional< std::is_same<newtype, void>::value, named_t< N, newtype >, void>::type;
	};
};

template < typename T >
struct has_name {
	template < typename U > static auto check(U u) -> decltype(U::name, std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template < symbol_t N, typename T >
constexpr auto check_name_if_has() -> std::enable_if_t<has_name<T>::value, bool> { return T::name == N; }
template < symbol_t N, typename T >
constexpr auto check_name_if_has() -> std::enable_if_t<!has_name<T>::value, bool> { return false; }

template < symbol_t N, typename T, size_t ... Pos, size_t ... Idx>
decltype(auto) lu_foreach_maybe_(std::index_sequence<Idx...>&&); // forward declaretion


/* �T���\�łȂ��^�ɑ΂��� lu_foreach_maybe_(index_sequence<>{}) ���Ăяo����,
   �߂�l�̐��_�̂��߂ɍċA����̂œ��ꉻ�Ŗ����I�ɔ����� */
template < bool compo, typename T, symbol_t N, size_t ...Pos > struct descend_if_traversable;
template < typename T, symbol_t N, size_t ...Pos >
struct descend_if_traversable < false, T, N, Pos... > {
	using type = decltype(std::make_index_sequence<0>());
};
template < typename T, symbol_t N, size_t ...Pos >
struct descend_if_traversable < true, T, N, Pos... > {
	using type = decltype(lu_foreach_maybe_< N, T, Pos... >(std::move(std::make_index_sequence<elementsof<T>()>())));
};

/* test_or_descend_ �� get<T, Idx> �� name/get �������ǂ����ŐU������ς���.
   4 �ʂ�̃R���r�l�[�V�����ɂȂ�̂ł��ꂼ��ꍇ�������� sub �֐��ɐU�� */
template < symbol_t N, typename T, size_t Idx, size_t ...Pos >
constexpr decltype(auto) test_or_descend_()
{
	using type = typename T::template get<0/*align*/, Idx>::type;
	constexpr bool r = check_name_if_has<N, type>();
	/* ��~���Ȃ��Ȃ� get<T, Pos..., Idx>::type �ɂ��ĒT������ */
	using Acc = std::index_sequence< Pos..., Idx >;
	return typename std::conditional< r, Acc, typename descend_if_traversable< has_get<type>::value, type, N, Pos..., Idx>::type >::type{};
}

/*
	optional<Acc2> r = false;
	auto v = {none, (r = (r || (test_or_descend_<N, T, Acc, Idx, Pos...>(std::move(acc)))))...};
	return r;
	�݂����Ȃ��Ƃ��ł���΂悩������, test_or_descend_ �̖߂�l�� bind ����̂����т�������
*/
template <symbol_t N, typename T, size_t ...Idx>
struct tod_loop_t { // end of Idx...
	template < size_t ...Pos > struct result {	using type = std::index_sequence<>;	};
};

template <symbol_t N, typename T, size_t Idx, size_t ...Rest>
struct tod_loop_t <N, T, Idx, Rest...> {
	template < size_t ...Pos >
	struct result {
		/* Acc �� loop �̌p���`�F�b�N�Ɏg����. �{���� Acc �� Pos..., Idx �̂ق� */
		using Acc = decltype(test_or_descend_<N, T, Idx, Pos...>());
		using type = typename std::conditional<Acc::size() != 0, Acc, typename tod_loop_t< N, T, Rest... >::template result<Pos...>::type >::type;
	};
};

/*
  lookup �p�� foreach_maybe ���`����.
  F �Ɍ^�p�����[�^��^�����Ȃ��Ƃ����Ȃ�����
  (�W�F�l���b�N�����_���������ꂽ C++20 �ȍ~�͂Ƃ�����
  foreach_maybe �ɗ^����N���[�W���̈����̌^��C�ӌ^�ɂ���̂͌����I�łȂ�.
  auto �͏����邪, �R���p�C�����v�Z�Ō^������ł��Ȃ���ΈӖ����Ȃ�)
*/ 
template < symbol_t N, typename T, size_t ... Pos, size_t ... Idx>
decltype(auto) lu_foreach_maybe_(std::index_sequence<Idx...>&&)
{
	using result = typename tod_loop_t< N, T, Idx...>::template result<Pos...>::type;
	return result{};
}

template < symbol_t N, typename T >
constexpr decltype(auto) lu_foreach_maybe()
{
	auto s = std::make_index_sequence<T::elementsof()>();
	return lu_foreach_maybe_<N, T>(std::move(s));
}

template < typename T, typename Pos, bool F >
struct lu_impl {
	using pos = Pos;
	using rawtype = typename decltype(T::inner_from_seq(pos{}))::type;
	using type = typename rawtype::type;

	static const bool found = true;

	static const size_t size = sizeof_<type, T::align_mode >();
	static const size_t offset = T::template offset_from(pos{});
	
	static inline constexpr type* addr(void* ptr) { return reinterpret_cast< type* >(reinterpret_cast<char*>(ptr) + offset); } 
};

template < typename T, typename Pos >
struct lu_impl< T, Pos, false > {
	static const bool found = false;
};

/*
  lu ���J�C���^�[�t�F�C�X

  symbol �� T �ɂ����, lu< T, symbol > ��:
  * �������^�� type, size
  * �����܂ł� offset, addr()
  ������.
  
  symbol ��������Ȃ��ꍇ, ��L�����o�������Ȃ�.
  ������Ȃ��ꍇ�ł�, found, name ������.
 */
template < typename T, symbol_t N, typename Pos = decltype(lu_foreach_maybe< N, T >()) >
struct lu : public lu_impl< T, Pos, Pos::size() != 0 > {
	static const symbol_t name = N;
};
}
#endif
