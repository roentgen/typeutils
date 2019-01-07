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
	template < typename U > static auto check(U u) -> decltype(U::align, std::true_type{});
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

template < alignment_t A, typename T>
constexpr auto decision_align() -> std::enable_if_t< has_align<T>::value, alignment_t > { return T::alignof_(); }

template < alignment_t A, typename T>
constexpr auto decision_align() -> std::enable_if_t< !has_align<T>::value, alignment_t > { return A; }

template < alignment_t A, typename T>
constexpr auto decision_align_value() -> std::enable_if_t< has_align<T>::value, alignment_t > { return T::align == 0 ? alignof_<T>() : T::align; }

template < alignment_t A, typename T>
constexpr auto decision_align_value() -> std::enable_if_t< !has_align<T>::value, alignment_t > { return A == 0 ? alignof(T) : A; }

template < typename T >
struct has_trv {
	template < typename U > static auto check(U u) -> decltype(u.trv(), std::true_type{}) { }
	static std::false_type check(...);
	static bool const value = decltype(check(std::declval<T>()))::value;
};

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

/* trv を持つ.
   trv はテンプレート引数に align を持つこと.
   (テンプレート引数はデフォルトの alignment を持つべき) */
template < typename T, alignment_t Align = 1 > constexpr auto sizeof_() -> decltype(T::trv())
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
template < typename T, alignment_t LastAlign, alignment_t Align = 1 >
constexpr auto sizeof_(size_t place) -> std::enable_if_t< has_placement<T, LastAlign>::value, size_t >
{
	return T::template placement< LastAlign, Align >(place);
}

template < typename T, alignment_t LastAlign, alignment_t Align = 1 >
constexpr auto sizeof_(size_t) -> std::enable_if_t< !has_placement<T, LastAlign>::value, size_t >
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

#if 1
template < typename Prev, alignment_t Align, size_t Acc, typename ...Ts >
struct sigma_size_impl {
	static const size_t value = Acc;
};
	
template < typename Prev, alignment_t Align, size_t Acc, typename CAR>
struct sigma_size_impl< Prev, Align, Acc, CAR > {
	static const alignment_t a0_ = decision_align_value< Align, CAR >();
	static const alignment_t a1_ = decision_align_value< Align, Prev >();
	static const size_t value = decision_align< Align, Prev >() != decision_align< Align, CAR >() ?
		align< a0_, CAR >(Acc + sizeof_< CAR, a1_, a0_ >(Acc)) :
		align< a0_, CAR >(Acc + sizeof_< CAR, a1_, a0_ >(Acc));
};

template < typename Prev, alignment_t Align, size_t Acc, typename CAR, typename ...CDR>
struct sigma_size_impl< Prev, Align, Acc, CAR, CDR... > {
	static const alignment_t a0_ = decision_align_value< Align, CAR >();
	static const alignment_t a1_ = decision_align_value< Align, Prev >();
	static const size_t value = decision_align< Align, Prev >() != decision_align< Align, CAR >() ?
		sigma_size_impl< CAR, a0_, align< a0_, CAR >(Acc + sizeof_< CAR, a1_, a0_ >(Acc)), CDR... >::value :
		sigma_size_impl< CAR, a0_, align< a0_, CAR >(Acc + sizeof_< CAR, a1_, a0_ >(Acc)), CDR... >::value;
};
#endif

	
template < alignment_t Align, size_t Acc, typename ...Ts >
struct sigma_size {
	static const size_t value = Acc;
};

/*
template < alignment_t Align, size_t Acc, typename CAR>
struct sigma_size< Align, Acc, CAR > {
	static const alignment_t a_ = decision_align< Align, CAR >();
	static const size_t value = align< Align, CAR >(Acc + sizeof_< CAR, a_, a_ >(Acc));
};
*/
template < alignment_t Align, size_t Acc, typename CAR, typename ...CDR>
struct sigma_size< Align, Acc, CAR, CDR... > {
	static const alignment_t a_ = decision_align< Align, CAR >();
	//static const size_t value = sigma_size_impl< CAR, a_, align< a_, CAR >(Acc + sizeof_< CAR, a_ , Align>(Acc)), CDR... >::value;
	static const size_t value = sigma_size< Align, align< a_, CAR >(Acc + sizeof_< CAR, a_, a_ >(Acc)), CDR... >::value;
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

	template < alignment_t Last, alignment_t Align = 1 >
	static constexpr size_t placement(size_t place = 0)
	{
		return trv<Align>(place);
	}
	
	template < alignment_t Align, size_t Acc, size_t... Rest > struct offset { static const size_t value = Acc; };

	/* offset は Idx-1 までのサイズの総和だが, address alignment 制約の保証のために Idx の type を知る必要がある.
	   計算の途中も制約を満たす必要がある.

	   FIXME:
	   type がもし type_t であって、アラインメントオーバーライドが行われるとしても, 
	   一回の sigma_type_list の呼び出しを通じては一貫して上位の制約が用いられるべきだが
	   なぜか逆でないと正しく動かない.
	   具体的には static const alignment_t a_ = decision_align< Align, T) >(); ではなく,
	   単に Align を用いるべきだ.
	*/
#if 1
	template < alignment_t Align, size_t Acc, size_t Cur >
	struct offset< Align, Acc, Cur > {
		/* 各 row 内の総和 */
		using T_ = typename at_type< Cur, S...>::type;
		static const alignment_t a_ = Align; //decision_align< Align, T_>();
		static const size_t value = align< Align, T_ >(sigma_type_list< a_, Acc >(typename to_types< Cur, S... >::types{}));
	};
#endif
	
	template < alignment_t Align, size_t Acc, size_t Cur, size_t ... Rest >
	struct offset< Align, Acc, Cur, Rest...>  {
		using T_ = typename at_type< Cur, S...>::type;
		static const alignment_t a_ = Align; //decision_align< Align, T_>();
		/* column ごとの総和: */
		static const size_t s = align< Align, T_ >(sigma_type_list< a_, Acc >(typename to_types< Cur, S... >::types{}));
		
		//static const size_t value = align< Align, T_ >(offset_< a_, s, T_, Rest... >());
		static const size_t value = align< Align, T_ >(offset_< Align, s, typename at_type< Cur, S... >::type, Rest... >());
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

	template < alignment_t Last, alignment_t Align = 1 >
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

/* 収容されるすべての型は固有のアラインメントを持つが,
   type_t はそれをオーバーライドする.
   一方, type_t 自身はアラインメントを持たない. 
   type_t は他の type_t に収容されるとき, その type_t のアラインメントにオーバーライドされる.

   これは, 上位 type_t がよりコンパクトなアラインを要求するときに要求に矛盾が生じる.
   using T1 = type_t< agg_t< double >, 0 >;
   using T0 = type_t< char, T2, 1>;
   とすると, T0 の収容型は
*/
template < typename S, alignment_t Align>
struct type_t < S, Align, std::enable_if_t< std::is_base_of< compo_t, S >::value > > {
	using sub = S;
	static const size_t align = Align;
	
	template < alignment_t A >
	static constexpr size_t align_(size_t o)
	{
		static_assert(A != 0, "align_ need not align-mode");
		size_t a = A; 
		size_t m = (a-1);
		return (o & m) ? ((o + m) & ~m): o;
	}

	/* type_t をネストして alignment を override するとき,
	   type_t の配置オフセットによって type_t 自身が pack を含む可能性がある */
	template < alignment_t EA = Align >
	constexpr static size_t trv(size_t placement = 0)
	{
		if (EA == Align)
			return sizeof_<sub, EA>();
		/* 自身の align と Enclosure の align が異なる場合,
		   自身の align で sizeof_ を計算し, placement を加えた offset を enclosure の align で計算する.
		 */
		size_t ma = typu::alignof_<sub>();
		/* effective alignment for the members */
		size_t iea = Align == 0 ? ma : Align;
		/* effective alignment for placement */
		size_t pea = EA == 0 ? ma : EA;
		
		size_t size = sizeof_<sub, Align>();
		/* ヘッドルームは,自身のアラインメント制約よりも大きなアラインメントに配置されるときに生じる.
		   ヘッドルームの大きさはコンテナの padding に含まれ, sizeof_<T> には含まれない. このため trv は sizeof_ とは異なる. */
		size_t offset = placement;
		/* TODO: 最大 leaf 要素の size をとる alignof_ が必要 */
		return align_< EA ? EA : typu::alignof_<sub>() >(offset) - placement + size;
		//return ((EA == 0) ? align_< typu::alignof_<sub>() >(offset) : align_< EA >(offset)) - placement + size;
	}

	/* type_t をネストして alignment を override するとき,
	   type_t の配置オフセットによって type_t 自身が pack を含む可能性がある */
	template < alignment_t LastAlign, alignment_t EA = Align >
	constexpr static size_t placement(size_t place = 0)
	{
		if (EA == Align) {
			auto r =  sizeof_<sub, 0, EA>(place);
			return r;
		}
		/* 自身の align と Enclosure の align が異なる場合,
		   自身の align で sizeof_ を計算し, placement を加えた offset を enclosure の align で計算する.
		 */
		size_t size = sizeof_<sub, 0, Align>(place);
		//return align_< constexpr_max((Align == 0 ? typu::alignof_<sub>() : Align),
		//							 (EA == 0 ? typu::alignof_<sub>() : EA)) >(place) - place + size;
		return align_< EA ? EA : (Align ? Align : typu::alignof_<sub>()) >(place) - place + size;
	}

	/* ignore the first 0 */
	template < size_t Cur, size_t ...Rest >
	constexpr static size_t offset_from()
	{
		return sub::template offset< Align, 0, Rest... >::value;
	}

	
	template < alignment_t A, size_t offs >
	constexpr static size_t check_aligned()
	{
		//static_assert((offs & ((A==0 ? sub::alignof_() : A) - 1)) == 0, "offset is not aligned");
		return offs;
	}

	static constexpr alignment_t alignof__() {return Align == 0 ? sub::alignof_() : Align;}

	template < alignment_t A, size_t Acc, size_t Pos, size_t ... Rest >
	struct offset {
		static const size_t value = check_aligned< A, sub::template offset< alignof__(), Acc, Rest... >::value >();
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

	static constexpr alignment_t alignof_()
	{
		return alignof__();
	}
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
	static const size_t offset = T::template offset_from<Pos...>();
};

}
#endif
