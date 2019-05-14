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

using namespace typu;

template < typename Acc, Acc Acc0, typename In >
struct fold_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_fun< Acc, Acc0, float[16] > { static const Acc value = Acc0 + 1; };

template < typename Acc, Acc Acc0, typename In >
struct fold_int_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_int_fun< Acc, Acc0, int > { static const Acc value = Acc0 + 1; };

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
	return 0;
}

 
