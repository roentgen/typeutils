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

template < alignment_t Align, typename T >
constexpr size_t align(size_t o)
{
	size_t a = Align; 
	if (Align == 0) /* natural align */
		a = sizeof(T);
	size_t m = (a-1);
	return (o & m) ? ((o + (a-1)) & ~m) : o;
}

/* trv を持つ.
   trv はテンプレート引数に align を持つこと.
   (テンプレート引数はデフォルトの alignment を持つべき) */
template < typename T, alignment_t Align = 1 > constexpr auto sizeof_() -> decltype(T::trv()) { return T::template trv< Align >(); }

/* trv を持たない */
template < typename T, alignment_t Align = 1 >
constexpr auto sizeof_() -> std::enable_if_t< !has_trv<T>::value, size_t >
{
	return align<Align, T>(sizeof(T));
}
	
template < alignment_t Align, size_t Acc, typename ...Ts >
struct sigma_size {
	static const size_t value = Acc;
};

template < alignment_t Align, size_t Acc, typename CAR>
struct sigma_size< Align, Acc, CAR > {
	static const size_t value = align< Align, CAR >(Acc + sizeof_< CAR >());
};

template < alignment_t Align, size_t Acc, typename CAR, typename ...CDR>
struct sigma_size< Align, Acc, CAR, CDR... > {
	static const size_t value = sigma_size< Align, align< Align, CAR >(Acc + sizeof_< CAR >()), CDR... >::value;
};

template < alignment_t Align, typename ...Ts >
constexpr size_t sigma_type_list(type_list< Ts ... >&&)
{
	return sigma_size< Align, 0, Ts... >::value;
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
	template < alignment_t Align = 1 >
	constexpr static size_t trv()
	{
		return sigma_size< Align, 0, S... >::value;
	}

	template < alignment_t Align, size_t... Rest > struct offset { static const size_t value = 0; };

	/* offset は Idx-1 までのサイズの総和だが, address alignment 制約の保証のために Idx の type を知る必要がある.
	   計算の途中も制約を満たす必要がある. */
	template < alignment_t Align, size_t Cur >
	struct offset< Align, Cur > { static const size_t value = align< Align, typename at_type< Cur, S...>::type >(sigma_type_list< Align >(typename to_types< Cur, S... >::types{})); };

	template < alignment_t Align, size_t Cur, size_t ... Rest >
	struct offset< Align, Cur, Rest...>  {
		static const size_t value = sigma_type_list< Align >(typename to_types< Cur, S... >::types{}) +
			offset_< Align, typename at_type< Cur, S... >::type, Rest... >();
	};

	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
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
	template < alignment_t Align = 1 >
	constexpr static size_t trv()
	{
		return constexpr_max(sizeof_<S, Align>()...);
	}

	template < alignment_t Align, size_t... Rest > struct offset { static const size_t value = 0; };
	template < alignment_t Align, size_t Pos >	struct offset< Align, Pos > { static const size_t value = 0; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct offset< Align, Pos, Rest...>  {
		static const size_t value = offset_< Align, typename at_type< Pos, S... >::type, Rest... >();
	};

	template < alignment_t Align, size_t... Rest > struct get { using type = void; };
	template < alignment_t Align, size_t Pos >	struct get< Align, Pos > { using type = typename at_type< Pos, S... >::type; };
	template < alignment_t Align, size_t Pos, size_t ... Rest >
	struct get< Align, Pos, Rest...>  {
		using type = typename at_type< Pos, S... >::type::template get< Align, Rest ... >::type;
	};
	
};

template < typename T, T ...S >
struct sel_t_t {
	template < alignment_t A=1 >
	constexpr static size_t trv()
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

template < typename S, alignment_t Align>
struct type_t < S, Align, std::enable_if_t< std::is_base_of< compo_t, S >::value > > {
	using sub = S;
	static const size_t align = Align;

	template < alignment_t A = Align >
	constexpr static size_t trv()
	{
		return sizeof_<sub, A>();
	}

	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset()
	{
		return sub::template offset< Align, Rest... >::value;
	}

	/* get インターフェイスのために compo_t の派生としたが, type_t 自身は compo_t を継承しない.
	   また variadic parameter もとらないため, 
	   get< type_t<...>, 0 > とやって sub にアクセスするためにはトリックが必要になった.
	   とりあえず特殊化で誤魔化す.
	   (そもそも最初の次元はもう要らないのではないかという気がする) */
	template < size_t ... Pos > struct inner;
	template < size_t Cur, size_t ...Rest >
	struct inner< Cur, Rest... > {
		using type = typename sub::template get< Align, Rest... >::type;
	};
	template < size_t Cur >
	struct inner <Cur> {
		using type = sub;
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
	static const size_t size = sizeof_<type, T::align >();
	static const size_t offset = T::template offset<Pos...>();
};

}
#endif
