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
struct has_alignof {
	template < typename U > static auto check(U u) -> decltype(U::alignof_(), std::true_type{});
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template < typename T >
constexpr auto alignof_() -> std::enable_if_t< has_alignof< T >::value, alignment_t >
{
	return T::alignof_();
}

template < typename T >
constexpr auto alignof_() -> std::enable_if_t< !has_alignof< T >::value, alignment_t >
{
	return alignof(T);
}

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

/* C++14 で std::max は constexpr 版ができたはずだが, clang-3.5+libc++ でなぜかだめ */
template < typename CAR, typename... CDR >
constexpr CAR constexpr_max(CAR&& car, CDR&&... cdr){
	CAR r = car;
	using s = std::initializer_list<int>;
	(void)s{ (void(r = r < cdr ? cdr : r),0)... };
	return r;
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
public:
	static bool const value = decltype(check(std::declval<T>()))::value;
};

/* offset を持たないので Pos... の探索は終了 */
template < alignment_t Align, size_t Acc, typename T, size_t...Pos >
constexpr auto offset_() -> std::enable_if_t< !has_offset<T, Align, Acc, Pos...>::value, size_t >
{
	return Acc + (std::is_empty< T >::value ? 0 : sizeof(T));
}

/* aggregation
 */
template < typename ...S >
struct agg_t : public compo_t {
	template < alignment_t Align = 1 >
	constexpr static size_t trv(size_t placement=0)
	{
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

	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
	};

	static constexpr alignment_t alignof_()
	{
		return constexpr_max(typu::alignof_<S>()...);
	}
};

/* selector */
template < typename ...S >
struct sel_t : public compo_t {
	template < alignment_t Align = 1 >
	constexpr static size_t trv(size_t placement=0)
	{
		return constexpr_max(sizeof_<S, Align>()...);
	}

	template < alignment_t Align = 1 >
	constexpr static size_t placement(size_t place=0)
	{
		return trv<Align>(place);
	}

	template < alignment_t Align, size_t Acc, size_t... Rest > struct offset { static const size_t value = Acc; };
	//template < alignment_t Align, size_t Acc, size_t Pos >	struct offset< Align, Acc, Pos > { static const size_t value = Acc; };
	template < alignment_t Align, size_t Acc, size_t Pos, size_t ... Rest >
	struct offset< Align, Acc, Pos, Rest...>  {
		static const size_t value = offset_< Align, Acc, typename at_type< Pos, S... >::type, Rest... >();
	};

	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
	};

	static constexpr alignment_t alignof_()
	{
		return constexpr_max(typu::alignof_<S>()...);
	}
};

template < typename T, T ...S >
struct sel_t_t {
	template < alignment_t A=1 >
	constexpr static size_t trv(size_t placement=0)
	{
		return constexpr_max(S...);
	}
};

/* 
   公開インターフェス get を使って offset, size にアクセスするのに struct get を持っている必要があるが,
   has_get のようなものを作って SFINAE でチェックするのが面倒になってきたので
   (get を使ってアクセスするのに)収容型は compo_t の派生であること、とした.

   TODO: が、これは失敗であるのでこの制限はなくしたほうがよい.
   compo_t に型アクセッサ get の実装を移せなければこのやり方には (単なる tag 以上の)意味はないが,
   get はインデクサ Pos... と派生先での収容型 S... の両方にアクセスするため難しい.
   compo_t を template にせざるを得ず、そうすると is_base_of で簡単に比較できなくなる.
 */
template < typename S, alignment_t Align, typename Enabled = void >
struct type_t {
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
struct type_t < S, Align, std::enable_if_t< std::is_base_of< compo_t, S >::value > > {
	using sub = S;
	static const alignment_t align_mode = Align; /* maybe 0 */
	static const size_t align = align_mode ? align_mode : sub::alignof_(); /* a concret value. non zero */
	
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

	template < alignment_t A, size_t... Rest > struct get { using type = void; };
	template < alignment_t A, size_t Pos >	struct get< A, Pos > { using type = sub; };
	template < alignment_t A, size_t Pos, size_t ... Rest >
	struct get< A, Pos, Rest...>  {
		using type = typename sub::template get< Align, Rest ... >::type;
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

	static constexpr alignment_t alignof_()	{ return align;	}

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

	static inline constexpr type* addr(void* ptr) { return reinterpret_cast< type* >(reinterpret_cast<char*>(ptr) + offset); }
};

}
#endif
