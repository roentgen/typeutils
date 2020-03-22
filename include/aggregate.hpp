/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#if !defined(TYPEUTIL_AGGREGATE_HPP__)
#define TYPEUTIL_AGGREGATE_HPP__
#include <utility>
#include <algorithm>
#include <type_traits>
#include <at.hpp>

namespace typu {

extern std::nullptr_t enabler;

using alignment_t = size_t;

template < typename T >
struct has_get {
	template < typename U > static auto check(U u) -> decltype(std::declval<typename U::template get<0>::type>(), std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};
template <> struct has_get< void > { static const bool value = false; };

/*
  enflat(Acc, [CAR|CDR]) ->
     enflat(Acc ++ enflat_if_traversable(Acc, CAR), CDR).

  enflat_if_traversable(Acc, T) when has_get(T) ->
     T:flatten(T, Acc);
	 Acc.

  T:flatten(Acc) -> enflat(Acc, S)
*/

template < bool compo, typename Acc, typename T > struct enflat_if_traversable;
template < typename Acc, typename T >
struct enflat_if_traversable < false, Acc, T > { using type = type_list<T>; };
template < typename Acc, typename T >
struct enflat_if_traversable < true, Acc, T > {	using type = typename T::flatten::template type< type_list<> >;};

template < typename Acc, typename ...Ts >
struct enflat { using type = Acc; };
template < typename Acc, typename CAR, typename ...CDR>
struct enflat< Acc, CAR, CDR... > {
	using T = decltype(Acc::concat(std::declval< typename enflat_if_traversable< has_get< CAR >::value, Acc, CAR >::type >()));
	using type = typename enflat< T, CDR... >::type;
};

template < typename ... S > 
struct compo_t  {
	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
	};

	struct flatten {
		template < typename Acc >
		using type = typename enflat< Acc, S... >::type;
	};
};

template < typename T >
struct has_fold {
	template <typename Acc, Acc Acc0, typename X>
	struct fun { static const Acc value = Acc0; };
	template < typename U > static auto check(U u) -> decltype(U::template fold<int, 0, fun >::value, std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template <> struct has_fold< void > { static const bool value = false; };

template < typename T >
struct has_mapped {
	template <typename Acc>
	struct fun { using type = Acc; };
	template < typename U > static auto check(U u) -> decltype(std::declval<typename U::template mapped< fun >::type >(), std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};
template <> struct has_mapped< void > { static const bool value = false; };

template < typename T >
constexpr auto elementsof() -> std::enable_if_t< !has_get<T>::value, size_t > { return 0; }

template < typename T >
constexpr auto elementsof() -> std::enable_if_t< has_get<T>::value, size_t > { return T::elementsof(); }

template < typename T >
constexpr auto countin() -> std::enable_if_t< !has_get<T>::value, size_t > { return std::is_same< T, void >::value ? 0 : 1; }

template < typename T >
constexpr auto countin() -> std::enable_if_t< has_get<T>::value, size_t > { return T::countin(); }

template < bool compo, typename T, template < typename... > class ...Ms > struct map_if_traversable;

template < typename T, template < typename... > class ...Ms >
struct map_if_traversable < false, T, Ms... > {
	using type = T;
};
template < typename T, template < typename... > class ...Ms >
struct map_if_traversable < true, T, Ms... > {
	using type = typename T::template mapped< Ms... >::type;
};

template < bool compo, typename T, template < typename... > class ...Ms > struct map_raw_if_traversable;

template < typename T, template < typename... > class ...Ms >
struct map_raw_if_traversable < false, T, Ms... > {
	using type = T;
};
template < typename T, template < typename... > class ...Ms >
struct map_raw_if_traversable < true, T, Ms... > {
	using type = typename T::template mapped< Ms... >::rawtype;
};

template < typename Acc, template < typename... > class ...Ms > struct apply_impl { using type = Acc; };
template < typename Acc, template < typename... > class M, template < typename... > class ...Ms >
struct apply_impl< Acc, M, Ms... > { using type = typename apply_impl< typename M< Acc >::type, Ms...>::type; };


template < typename Arg, typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Ms > struct apply2_impl {
	using type = Arg;
	static const Acc value = Acc0;
};

template < typename Arg, typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename M, template < typename Acc_, Acc_, typename... > typename ...Ms >
struct apply2_impl< Arg, Acc, Acc0, M, Ms... > {
	using type = Arg;
	static const Acc value = M<Acc, Acc0, Arg>::value;
};


template < template < typename... > class F, typename Acc, typename ...Ts > struct filter_impl { using type = Acc; };
template < template < typename... > class F, typename Acc, typename T, typename ...Ts >
struct filter_impl< F, Acc, T, Ts... > { using type = typename filter_impl< F, typename std::conditional< F<T>::value, typename Acc::template cons< T >, Acc >::type, Ts...>::type; };

template < typename T >
struct not_void {
	static const bool value = !(std::is_same< T, void >::value);
};

template < typename T >
struct not_empty_composite {
	static const bool value = ((!has_get<T>::value) || elementsof<T>() > 0);
};

/* 自己言及的に型を返す必要があるが template template parameter に不完全型を与えるのが clang5.0 までダメ */
template < template < typename ... > class T, typename ...S >
struct map_t {
	template < template < typename... > class ...M > struct mapped {
		template < template <typename> class F, typename ...Ts >
		static constexpr auto filter(type_list<Ts...>&&) -> typename filter_impl< F, type_list<>, Ts... >::type;

		using rawtypelist = type_list< typename apply_impl< typename map_raw_if_traversable< has_mapped<S>::value, S, M... >::type, M... >::type ... >;
		using rawtype = typename rawtypelist::template rewrap_t< T >;

		using typelist = type_list< typename apply_impl< typename map_if_traversable< has_mapped<S>::value, S, M... >::type, M... >::type ... >;
		using filtered = decltype(filter< not_empty_composite >(filter< not_void >(typelist{})));
		using type = typename filtered::template rewrap_t< T >;
	};
};

template < bool compo, typename T, typename Acc, Acc acc0, template < typename Acc_, Acc_, typename... > typename ...Fun > struct fold_if_traversable;
template < typename T, typename Acc, Acc acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
struct fold_if_traversable < false, T, Acc, acc0, Fun... > {
	static const Acc value = acc0;
};
template < typename T, typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
struct fold_if_traversable < true, T, Acc, Acc0, Fun... > {
	static const Acc value = T::template fold< Acc, Acc0, Fun... >::value;
};

template < typename Acc_, Acc_ Acc0_, typename ...CDR>
struct extract_S {
	template <template < typename Acc, Acc, typename... > typename ...>
	static constexpr Acc_ eval() { return Acc0_; };
};
		
template < typename Acc_, Acc_ Acc0, typename CAR, typename ...CDR>
struct extract_S< Acc_, Acc0, CAR, CDR...> {
	template <template < typename Acc, Acc, typename... > typename ...Fun>
	static constexpr Acc_ eval() {
		return apply2_impl< CAR, Acc_,
							fold_if_traversable< has_fold<CAR>::value, CAR, Acc_,
												 extract_S<Acc_, Acc0, CDR... >::template eval<Fun...>(),
												 Fun... >::value,
							Fun... >::value;
	}
};


/* 収容型 S... に対して fold(acc0, S) を実行する. */
template < template < typename ... > class T, typename ...S >
struct fold_t {
	template < typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
	struct fold {
		static const Acc value = extract_S< Acc, Acc0, S... >::template eval<Fun...>();
	};
};

template < typename Out > struct base_mapper { using type = Out; };
template < typename In > struct mapto { using type = In; };

template < typename T >
struct has_alignof {
	template < typename U > static constexpr auto check(U u) -> decltype(U::alignof_(), std::true_type{});
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template <> struct has_alignof<void> { static const bool value = false; };

template < typename T >
constexpr auto alignof_() -> std::enable_if_t< has_alignof< T >::value, alignment_t >
{
	return T::alignof_();
}

template < typename T >
constexpr auto alignof_() -> std::enable_if_t< !std::is_same<void, T>::value && !has_alignof< T >::value, alignment_t >
{
	return alignof(T);
}

template < typename T >
constexpr auto alignof_() -> std::enable_if_t< std::is_same<void, T>::value && !has_alignof< T >::value, alignment_t > { return 0; }


template < typename T>
struct has_align {
	template < typename U > static auto check(U u) -> decltype(U::align_mode, std::true_type{});
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template < alignment_t A, typename T>
constexpr auto decision_align() -> std::enable_if_t< has_align<T>::value, alignment_t > { return T::alignof_(); }

template < alignment_t A, typename T>
constexpr auto decision_align() -> std::enable_if_t< !has_align<T>::value, alignment_t > { return A; }

template < alignment_t Align, typename T >
constexpr size_t align(size_t o)
{
	size_t a = Align; 
	if (Align == 0) /* natural align */
		a = alignof_<T>();
	size_t m = (a-1);
	return (o & m) ? ((o + (a-1)) & ~m) : o;
}

template < alignment_t Align >
constexpr size_t align(size_t o)
{
	size_t a = Align; 
	if (Align == 0) /* natural align */
		return o;
	size_t m = (a-1);
	return (o & m) ? ((o + (a-1)) & ~m) : o;
}


template < typename T >
struct has_trv {
	template < typename U > static auto check(U u) -> decltype(u.trv(), std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

/* trv を持つ.
   trv はテンプレート引数に align を持つこと.
   (テンプレート引数はデフォルトの alignment を持つべき) */
template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_() -> std::enable_if_t< has_trv<T>::value, size_t >
{
	return T::template trv< Align >();
}

/* trv を持たない */
template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_() -> std::enable_if_t< !has_trv<T>::value, size_t >
{
	return align<Align, T>(sizeof(T));
}

template < typename T, alignment_t A >
struct has_placement {
	template < typename U > static auto check(U u) -> decltype(U::template placement<A>(), std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

/* placement を考慮する size 計算を呼び出す. */
template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_(size_t place) -> std::enable_if_t< has_placement<T, Align>::value, size_t >
{
	return T::template placement< Align >(place);
}

template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_(size_t) -> std::enable_if_t< !has_placement<T, Align>::value, size_t >
{
	return align<Align, T>(sizeof(T));
}
	
template < alignment_t Align, size_t Acc, typename ...Ts >
struct sigma_size {
	static const size_t value = Acc;
};

template < alignment_t Align, size_t Acc, typename CAR, typename ...CDR>
struct sigma_size< Align, Acc, CAR, CDR... > {
	static const alignment_t a_ = decision_align< Align, CAR >();
	static const size_t value = sigma_size< Align, align< a_, CAR >(Acc + sizeof_< CAR, a_ >(Acc)), CDR... >::value;
};

template < alignment_t Align, size_t Acc, typename ...Ts >
constexpr size_t sigma_type_list(type_list< Ts ... >&&)
{
	return sigma_size< Align, Acc, Ts... >::value;
}

template < alignment_t Align, size_t Acc, typename Cur, typename ...Ts >
constexpr size_t sigma_type_list(type_list< Ts ... >&&)
{
	return sigma_size< Align, Acc, Cur, Ts... >::value;
}


/* offset 計算のサブ関数:
   型 T が offset<Rest...>() を持っていれば Rest... のパターンに従って探索し, なければ終端する. */
template < alignment_t Align, size_t Acc, typename T, size_t...Pos >
constexpr auto offset_() -> decltype(T::template offset< Align, Acc, Pos... >::value)
{
	return T::template offset< Align, Acc, Pos... >::value ;
}

template < typename T, alignment_t A, size_t Acc, size_t... Pos >
struct has_offset {
	template < typename U > static auto check(U u) -> decltype(U::template offset< A, Acc, Pos ... >::value, std::true_type{}) { }
	static std::false_type check(...);
	static const bool value = decltype(check(std::declval<T>()))::value;
};

/* offset を持たないので Pos... の探索は終了 */
template < alignment_t Align, size_t Acc, typename T, size_t...Pos >
constexpr auto offset_() -> std::enable_if_t< !has_offset<T, Align, Acc, Pos...>::value, size_t >
{
	return Acc + (std::is_empty< T >::value ? 0 : sizeof(T));
}

template < size_t Acc, typename ...Ts >
struct count_leaf_t { static const size_t value = Acc; };

template < size_t Acc, typename CAR, typename ...CDR>
struct count_leaf_t< Acc, CAR, CDR... > {
	static const size_t value = count_leaf_t< Acc + countin<CAR>(), CDR... >::value;
};

template < size_t Acc, typename ...Ts >
constexpr size_t count_leaf_type_list(type_list< Ts ... >&&) { return count_leaf_t< Acc, Ts... >::value; }

template < size_t Acc, typename Cur, typename ...Ts >
constexpr size_t count_leaf_type_list(type_list< Ts ... >&&) { return count_leaf_t< Acc, Cur, Ts... >::value; }

template < typename T, size_t Acc, size_t... Pos >
struct has_leaves {
	template < typename U > static auto check(U u) -> decltype(U::template leaves< Acc, Pos ... >::value, std::true_type{}) {}
	static std::false_type check(...);
	static const bool value = decltype(check(std::declval<T>()))::value;
};

/* leaves 計算のサブ関数:
   型 T が leaves<Rest...>() を持っていれば Rest... のパターンに従って探索し, なければ終端する. */
template < size_t Acc, typename T, size_t...Pos >
constexpr auto leaves_by_index() -> decltype(T::template leaves< Acc, Pos... >::value)
{
	return T::template leaves< Acc, Pos... >::value ;
}

/* leaves を持たないので Pos... の探索は終了 */
template < size_t Acc, typename T, size_t...Pos >
constexpr auto leaves_by_index() -> std::enable_if_t< !has_leaves<T, Acc, Pos...>::value, size_t >
{
	return Acc + (std::is_empty< T >::value ? 0 : 1);
}

/* aggregation
 */
template < typename ...S >
struct agg_t : public compo_t< S... >, public map_t< agg_t, S... >, fold_t< agg_t, S... > {
	template < alignment_t Align = 1 >
	constexpr static size_t trv(size_t placement=0)
	{
		(void)placement;
		return sigma_size< Align, 0, S... >::value;
	}

	template < alignment_t Align = 1 >
	static constexpr size_t placement(size_t place = 0)
	{
		return trv<Align>(place);
	}
	
	template < alignment_t Align, size_t Acc, size_t... Rest > struct offset { static const size_t value = Acc; };

	/* offset は Idx-1 までのサイズの総和だが, address alignment 制約の保証のために Idx の type を知る必要がある.
	   計算の途中も制約を満たす必要がある.
	   このときの alignment は placement alignment で, 型の要求 alignment に優先する.
	   (alignment がオーバーライドされるとき, 内側の type_t の align より placement-align が優先される.
	    ただし type_t の内部は type_t の align が適用される. この padding/headroom を type_t が持つ.
		headroom は type_t が直接収容する agg_t/sel_t の offset をとれば計算済みとなる)
		using T = type_t< agg_t< char, type_t< agg_t< int >, 0 >, 1 > のとき:
		get< T, 0, 1 >::offset は placement-alignment=1 に従い 1
		get< T, 0, 1, 0>::offset は T の内部 alignment に従い 4
		get< T, 0, 1, 0, 0>::offset は内部の alignment に従い 4
	*/
#if 1
	template < alignment_t Align, size_t Acc, size_t Cur >
	struct offset< Align, Acc, Cur > {
		/* 最終 column の 総和. 最後の要素の加算前の値を返す */
		using T_ = typename at_type< Cur, S...>::type;
		static const size_t value = align< Align, T_ >(sigma_type_list< Align, Acc >(typename to_types< Cur, S... >::types{}));
	};
#endif
	
	template < alignment_t Align, size_t Acc, size_t Cur, size_t ... Rest >
	struct offset< Align, Acc, Cur, Rest...>  {
		using T_ = typename at_type< Cur, S...>::type;
		/* column ごとの総和: */
		static const size_t s = align< Align, T_ >(sigma_type_list< Align, Acc >(typename to_types< Cur, S... >::types{}));
		static const size_t value = align< Align, T_ >(offset_< Align, s, T_, Rest... >());
	};

	/* leaves of Pos... */
	template < size_t Acc, size_t... Rest > struct leaves { static const size_t value = Acc; };

	template < size_t Acc, size_t Cur, size_t ... Rest >
	struct leaves< Acc, Cur, Rest...>  {
		using T_ = typename at_type< Cur, S...>::type;
		static const size_t value = leaves_by_index< count_leaf_type_list< Acc >(typename to_types< Cur, S... >::types{}), T_, Rest... >();
	};
	
	static constexpr size_t elementsof() { return sizeof...(S); };
	static constexpr alignment_t alignof_()	{ return std::max(std::initializer_list<size_t>{typu::alignof_<S>()...});	}
	static constexpr size_t countin() { return count_leaf_t<0, S...>::value; }
};

/* selector */
template < typename ...S >
struct sel_t : public compo_t< S... >, public map_t< sel_t, S... > {
	template < alignment_t Align = 1 >
	constexpr static size_t trv(size_t placement=0)
	{
		(void)placement;
		return std::max(std::initializer_list<size_t>{sizeof_<S, Align>()...});
	}

	template < alignment_t Align = 1 >
	constexpr static size_t placement(size_t place=0)
	{
		return trv<Align>(place);
	}

	template < alignment_t Align, size_t Acc, size_t... Rest > struct offset { static const size_t value = Acc; };
	template < alignment_t Align, size_t Acc, size_t Pos >	struct offset< Align, Acc, Pos > { static const size_t value = Acc; };
	template < alignment_t Align, size_t Acc, size_t Pos, size_t ... Rest >
	struct offset< Align, Acc, Pos, Rest...>  {
		static const size_t value = offset_< Align, Acc, typename at_type< Pos, S... >::type, Rest... >();
	};

	/* leaves of Pos... */
	template < size_t Acc, size_t... Rest > struct leaves { static const size_t value = Acc; };

	template < size_t Acc, size_t Cur, size_t ... Rest >
	struct leaves< Acc, Cur, Rest...>  {
		using T_ = typename at_type< Cur, S...>::type;
		static const size_t value = leaves_by_index< count_leaf_type_list< Acc >(type_list<T_>{}), T_, Rest... >();
	};
	
	static constexpr size_t elementsof() { return sizeof...(S); };
	static constexpr alignment_t alignof_()	{ return std::max(std::initializer_list<size_t>{typu::alignof_<S>()...});	}
	static constexpr size_t countin() { return std::max(std::initializer_list<size_t>{count_leaf_t<0, S>::value...}); }
};

	
/* 収容されるすべての型は固有のアラインメントを持ち, type_t はそれをオーバーライドする.
   type_t は他の type_t に収容されるとき, その type_t のアラインメントにオーバーライドされる.

   これは, 上位 type_t がよりコンパクトなアラインを要求するときに要求に矛盾が生じる.
   using T1 = type_t< agg_t< double >, 0 >;
   using T0 = type_t< char, T2, 1>;
   とすると, T0 の収容型は packed となり T1 も offset=1 となるが,
   T1 の収容型は自然な align を要求されるので type_t が 7byte の headroom となり
   T1 収容型のオフセットは 8byte align となる.
*/
template < typename S, alignment_t Align>
struct type_t : public compo_t< S > {
	using sub = S;
	static const alignment_t align_mode = Align; /* maybe 0 */
	static const size_t align = align_mode ? align_mode : alignof_<sub>(); /* a concret value. non zero */
	
	template < alignment_t A >
	static constexpr size_t align_(size_t o)
	{
		static_assert(A != 0, "align_ needs not align-mode");
		size_t a = A; 
		size_t m = (a-1);
		return (o & m) ? ((o + m) & ~m): o;
	}

	/* type_t をネストして alignment を override するとき,
	   type_t の配置オフセットによって type_t 自身が pack を含む可能性がある */
	template < alignment_t EA = Align >
	constexpr static size_t trv(size_t place = 0)
	{
		if (EA == Align)
			return sizeof_<sub, EA>();
		/* 自身の align と Enclosure の align が異なる場合,
		   自身の align で sizeof_ を計算し, placement を加えた offset を enclosure の align で計算する.
		 */
		size_t size = sizeof_<sub, Align>();
		return align_< EA ? EA : align >(place) - place + size;
	}

	/* type_t をネストして alignment を override するとき,
	   type_t の配置オフセットによって type_t 自身が pack を含む可能性がある */
	template < alignment_t EA = Align >
	constexpr static size_t placement(size_t place = 0)
	{
		if (EA == Align) 
			return sizeof_<sub, EA>(place);
		/* 自身の align と Enclosure の align が異なる場合,
		   自身の align で sizeof_ を計算し, placement を加えた offset を enclosure の align で計算する.
		 */
		size_t size = sizeof_<sub, Align>(place);
		return align_< EA ? EA : align >(place) - place + size;
	}

	/* ignore the first 0 */
	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset_from()
	{
		static_assert(Cur == 0, "type_t has to get 0 for the first enclosure");
		return sub::template offset< Align, 0, Rest... >::value;
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset_from(std::index_sequence<Cur, Rest...>&&)
	{
		return offset_from< Cur, Rest... >();
	}

	/* ignore the first 0 */
	template < size_t Cur, size_t ...Rest >
	constexpr static size_t index_from()
	{
		static_assert(Cur == 0, "type_t has to get 0 for the first enclosure");
		size_t n = sub::template leaves< 0, Rest... >::value;
		return n ? n - 1 : 0;
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t index_from(std::index_sequence<Cur, Rest...>&&)
	{
		return index_from< Cur, Rest... >();
	}

	template < alignment_t A, size_t offs >
	constexpr static size_t check_aligned()
	{
		static_assert((offs & ((A==0 ? sub::alignof_() : A) - 1)) == 0, "offset is not aligned");
		return offs;
	}

	template < alignment_t A, size_t Acc, size_t Pos, size_t ... Rest >
	struct offset {
		/* type_t の内部 offset の計算にしか呼ばれないので上位 placement align は無視 */
		static const size_t value = align_< align >(offset_< align, Acc, sub, Rest... >());
	};

	template < size_t Acc, size_t Pos, size_t ... Rest >
	struct leaves {
		static const size_t value = leaves_by_index< Acc, sub, Rest... >();
	};

	/* get インターフェイスのために compo_t の派生としたが, type_t 自身は compo_t を継承しない.
	   また variadic parameter もとらないため, 
	   get< type_t<...>, 0 > とやって sub にアクセスするためにはトリックが必要になった.
	   とりあえず特殊化で誤魔化す.
	   (そもそも最初の次元はもう要らないのではないかという気がするが type_t は alignment override の際境界となる) */
	template < size_t ... Pos > struct inner;
	template < size_t Cur, size_t ...Rest >
	struct inner< Cur, Rest... > {
		using type = typename sub::template get< Align, Rest... >::type;
	};
	template < size_t Cur >
	struct inner <Cur> {
		using type = sub;
	};

	template < size_t ...Pos >
	static constexpr auto inner_from_seq(std::index_sequence< Pos... >&&) -> inner<Pos...>;// {return std::declval< inner<Pos...> >();};

	/* elementsof() は直接収容する型の数を返す. countin() は再帰的に記憶領域を持つ全 leaf を数える. void と(node になる)収容型は数えない.
	   このため countin() は sel_t では max<countin(S)...> を返す.
	   elementsof() returns num of types which the type includes directly.
	   countin() recursively counts all leaves that has its storage up. thus, void and comp_t whose don't have its storage are ignored.
	*/
	static constexpr size_t elementsof() { return 1; };
	static constexpr alignment_t alignof_()	{ return align;	}
	static constexpr size_t countin() { return S::countin(); }

	template < template < typename... > class ...M > struct mapped {
		template < typename ...Ts >
		static constexpr auto filter(type_list<Ts...>&&) -> typename filter_impl< not_void, type_list<>, Ts... >::type;
		
		using rawtypelist = type_list< typename apply_impl< typename map_raw_if_traversable< has_mapped<S>::value, S, M... >::type, M... >::type >;
		using rawtype = type_t< typename at_types< 0, rawtypelist >::type, Align >;

		using typelist = type_list< typename apply_impl< typename map_if_traversable< has_mapped<S>::value, S, M... >::type, M... >::type >;
		using type = type_t< typename at_types< 0, decltype(filter(typelist{})) >::type, Align >;
	};

	template < typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
	struct fold {
		static constexpr Acc eval()
		{
			return apply2_impl< S, Acc, fold_if_traversable< has_fold<S>::value, S, Acc, Acc0, Fun... >::value, Fun... >::value;
		}
	};
};


template < alignment_t Align>
struct type_t<void, Align> : public compo_t< void > {
	using sub = void;
	static const alignment_t align_mode = Align; /* maybe 0 */
	static const size_t align = align_mode;

	template < alignment_t EA = Align >
	constexpr static size_t trv(size_t place = 0) { return Align; }

	template < alignment_t EA = Align >
	constexpr static size_t placement(size_t place = 0) { return Align; }

	/* ignore the first 0 */
	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset_from()
	{
		static_assert(Cur == 0, "type_t has to get 0 for the first enclosure");
		return 0;
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset_from(std::index_sequence<Cur, Rest...>&&) { return offset_from< Cur, Rest... >(); }

	/* ignore the first 0 */
	template < size_t Cur, size_t ...Rest >
	constexpr static size_t index_from()
	{
		static_assert(Cur == 0, "type_t has to get 0 for the first enclosure");
		return 0;
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t index_from(std::index_sequence<Cur, Rest...>&&) { return index_from< Cur, Rest... >(); }

	template < alignment_t A, size_t offs >
	constexpr static size_t check_aligned() { return offs; }

	template < alignment_t A, size_t Acc, size_t Pos, size_t ... Rest >
	struct offset { static const size_t value = 0; };

	template < size_t Acc, size_t Pos, size_t ... Rest >
	struct leaves { static const size_t value = 0; };

	template < size_t ... Pos > struct inner { using type = sub; };

	template < size_t ...Pos >
	static constexpr auto inner_from_seq(std::index_sequence< Pos... >&&) -> inner<Pos...>;

	static constexpr size_t elementsof() { return 0; };
	static constexpr alignment_t alignof_()	{ return align;	}
	static constexpr size_t countin() { return 0; }

	/* void でも map/fold は動かす */
	template < template < typename... > class ...M > struct mapped {
		template < typename ...Ts >
		static constexpr auto filter(type_list<Ts...>&&) -> typename filter_impl< not_void, type_list<>, Ts... >::type;

		using rawtypelist = type_list< typename apply_impl< typename map_raw_if_traversable< has_mapped<sub>::value, sub, M... >::type, M... >::type >;
		using rawtype = type_t< typename at_types< 0, rawtypelist >::type, Align >;

		using typelist = type_list< typename apply_impl< typename map_if_traversable< has_mapped<sub>::value, sub, M... >::type, M... >::type >;
		using type = type_t< typename at_types< 0, decltype(filter(typelist{})) >::type, Align >;
	};

	template < typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename ...Fun >
	struct fold {
		static constexpr Acc eval() { return apply2_impl< sub, Acc, fold_if_traversable< has_fold<sub>::value, sub, Acc, Acc0, Fun... >::value, Fun... >::value; }
	};
};


/* get 公開インターフェイス.
   これを使うには, type_t が comp_t 派生型を直接所有していること.
   (また comp_t 派生型は struct get を保持して <0, 2, ... > のような variadic parameter で
   要素を特定できるようになっている必要がある.)
 */
template < typename T, size_t... Pos >
struct get {
	using type = typename T::template inner< Pos... >::type;
	static const size_t size = sizeof_<type, T::align_mode >();
	static const size_t offset = T::template offset_from<Pos...>();
	static const size_t index = T::template index_from<Pos...>();

	static inline constexpr type* addr(void* ptr) { return reinterpret_cast< type* >(reinterpret_cast<char*>(ptr) + offset); }
};

template < typename T, template < typename... > class...Ms >
struct morph {
	using mapped = typename T::template mapped< Ms... >::type;
	using mapped_raw = typename T::template mapped< Ms... >::rawtype;
};

template < typename T, template < typename... > class...Ms >
using mapped_t = typename morph<T, Ms...>::mapped;

template < typename T, typename Acc, Acc Acc0, template < typename Acc_, Acc_, typename... > typename... Fun >
struct fold {
	static const Acc value = T::template fold< Acc, Acc0, Fun... >::eval();
};

template < typename T >
struct flatten {
	using type = typename T::flatten::template type< type_list<> >;
};

template < typename T > using flatten_t = typename flatten< T >::type;

}
#endif
