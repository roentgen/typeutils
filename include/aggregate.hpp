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

/* trv ������.
   trv �̓e���v���[�g������ align ��������.
   (�e���v���[�g�����̓f�t�H���g�� alignment �����ׂ�) */
template < typename T, alignment_t Align = 1 > constexpr auto sizeof_() -> decltype(T::trv()) { return T::template trv< Align >(); }

/* trv �������Ȃ� */
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

/* offset �v�Z�̃T�u�֐�:
   �^ T �� offset<Rest...>() �������Ă���� Rest... �̃p�^�[���ɏ]���ĒT����, �Ȃ���ΏI�[����. */
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

/* offset �������Ȃ��̂� Pos... �̒T���͏I�� */
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

	/* offset �� Idx-1 �܂ł̃T�C�Y�̑��a����, address alignment ����̕ۏ؂̂��߂� Idx �� type ��m��K�v������.
	   �v�Z�̓r��������𖞂����K�v������. */
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

/* C++14 �� std::max �� constexpr �ł��ł����͂�����, clang-3.5+libc++ �łȂ������� */
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
   ���J�C���^�[�t�F�X get ���g���� offset, size �ɃA�N�Z�X����̂� struct get �������Ă���K�v�����邪,
   has_get �̂悤�Ȃ��̂������ SFINAE �Ń`�F�b�N����̂��ʓ|�ɂȂ��Ă����̂�
   (get ���g���ăA�N�Z�X����̂�)���e�^�� compo_t �̔h���ł��邱�ƁA�Ƃ���.

   TODO: ���A����͎��s�ł���̂ł��̐����͂Ȃ������ق����悢.
   compo_t �Ɍ^�A�N�Z�b�T get �̎������ڂ��Ȃ���΂��̂����ɂ� (�P�Ȃ� tag �ȏ��)�Ӗ��͂Ȃ���,
   get �̓C���f�N�T Pos... �Ɣh����ł̎��e�^ S... �̗����ɃA�N�Z�X���邽�ߓ��.
   compo_t �� template �ɂ�����𓾂��A��������� is_base_of �ŊȒP�ɔ�r�ł��Ȃ��Ȃ�.
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

	/* get �C���^�[�t�F�C�X�̂��߂� compo_t �̔h���Ƃ�����, type_t ���g�� compo_t ���p�����Ȃ�.
	   �܂� variadic parameter ���Ƃ�Ȃ�����, 
	   get< type_t<...>, 0 > �Ƃ���� sub �ɃA�N�Z�X���邽�߂ɂ̓g���b�N���K�v�ɂȂ���.
	   �Ƃ肠�������ꉻ�Ō떂����.
	   (���������ŏ��̎����͂����v��Ȃ��̂ł͂Ȃ����Ƃ����C������) */
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

/* get ���J�C���^�[�t�F�C�X.
   ������g���ɂ�, type_t �� comp_t �h���^�𒼐ڏ��L���Ă��邱��.
   (�܂� comp_t �h���^�� struct get ��ێ����� <0, 2, ... > �̂悤�� variadic parameter ��
   �v�f�����ł���悤�ɂȂ��Ă���K�v������.)
 */
template < typename T, size_t... Pos >
struct get {
	using type = typename T::template inner< Pos... >::type;
	static const size_t size = sizeof_<type, T::align >();
	static const size_t offset = T::template offset<Pos...>();
};

}
#endif
