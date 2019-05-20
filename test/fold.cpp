#include <stdio.h>
#include <aggregate.hpp>
#include <lookup.hpp>
#include <str.hpp>
#include <type_traits>
#include <typeinfo>
#include <stdint.h>
#if defined(HAS_CXXABI_H)
#include <cxxabi.h>
#endif

struct matrix { float m[16]; };

struct non_trivial_t {
	int x;
	non_trivial_t() = delete;
};

using namespace typu;

template < typename Acc, Acc Acc0, typename In >
struct fold_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_fun< Acc, Acc0, float[16] > { static const Acc value = Acc0 + 1; };

template < typename Acc, Acc Acc0, typename In >
struct fold_int_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_int_fun< Acc, Acc0, int > { static const Acc value = Acc0 + 1; };

template < typename Acc, Acc Acc0, typename In >
struct fold_nt_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_nt_fun< Acc, Acc0, non_trivial_t > { static const Acc value = Acc0 + 1; };

const char* demangle(const char* nm)
{
#if defined(HAS_CXXABI_H)
	int status;
	return abi::__cxa_demangle(nm, 0, 0, &status);
#else
	return nm;
#endif
}

int main()
{
	using T1 = type_t< agg_t< int, int, float[16], char >, 0 >;
	printf("T1 -> %s : %d float[16]\n", demangle(typeid(T1).name()), fold< T1, int, 0, fold_fun >::value);
	printf("T1 -> %s : %d int\n", demangle(typeid(T1).name()), fold< T1, int, 0, fold_int_fun >::value);

	using T2 = type_t< agg_t< int, agg_t< int >, agg_t< float[16] >, char >, 0 >;
	printf("T2 -> %s : %d float[16]\n", demangle(typeid(T2).name()), fold< T2, int, 0, fold_fun >::value);
	printf("T2 -> %s : %d int\n", demangle(typeid(T2).name()), fold< T2, int, 0, fold_int_fun >::value);

	using T3 = type_t< agg_t< int, named_t< 1, agg_t< int > >, named_t< 2, float[16] >, char >, 0 >;
	printf("T3 -> %s : %d float[16]\n", demangle(typeid(T3).name()), fold< T3, int, 0, fold_fun >::value);
	printf("T3 -> %s : %d int\n", demangle(typeid(T3).name()), fold< T3, int, 0, fold_int_fun >::value);

	using T4 = type_t< agg_t< int, named_t< 1, agg_t< non_trivial_t > >, named_t< 2, float[16] >, char >, 0 >;
	printf("T4 -> %s : %d float[16]\n", demangle(typeid(T4).name()), fold< T4, int, 0, fold_fun >::value);
	printf("T4 -> %s : %d non_trivial_t\n", demangle(typeid(T4).name()), fold< T4, int, 0, fold_nt_fun >::value);

	return 0;
}

 
