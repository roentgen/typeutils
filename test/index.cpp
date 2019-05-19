#include <stdio.h>
#include <aggregate.hpp>
#include <lookup.hpp>
#include <type_traits>
#include <typeinfo>
#include <stdint.h>
#if defined(HAS_CXXABI_H)
#include <cxxabi.h>
#endif

struct matrix { float m[16]; };

using namespace typu;

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
	using T0 = type_t< agg_t< int, int, int, int, float, double >, 0 >;
	printf("T0 -> %s : %s\n", demangle(typeid(T0).name()), demangle(typeid(get<T0, 0, 4 >::type).name()));
	printf("T0 -> %s : %zu\n", demangle(typeid(T0).name()), get<T0, 0, 4 >::index);

	using T1 = type_t< agg_t< int, int, agg_t< bool, bool, bool >, float[16], agg_t<int, int, float, double>, char >, 0 >;
	printf("T1 -> %s : %s\n", demangle(typeid(T1).name()), demangle(typeid(get<T1, 0, 4, 2 >::type).name()));
	printf("T1 -> %s : %zu\n", demangle(typeid(T1).name()), get<T1, 0, 4, 2 >::index);

	using T2 = type_t< agg_t< int, int, type_t< agg_t< agg_t< bool, bool, bool > >, 8>, float[16], agg_t<int, int, float, double>, char >, 0 >;
	printf("T2 -> %s : %s\n", demangle(typeid(T2).name()), demangle(typeid(get<T2, 0, 4, 2 >::type).name()));
	printf("T2 -> %s : %zu\n", demangle(typeid(T2).name()), get<T2, 0, 4, 2 >::index);

	using T3 = type_t< agg_t< int, int, type_t< agg_t< sel_t< bool, bool, bool, bool > >, 8>, float[16], sel_t<int, int, float, double>, char >, 0 >;
	printf("T3 -> %s : %s\n", demangle(typeid(T3).name()), demangle(typeid(get<T3, 0, 4, 2 >::type).name()));
	printf("T3 -> %s : %zu\n", demangle(typeid(T3).name()), get<T3, 0, 4, 2 >::index);

	using T4 = type_t< agg_t< int, int, type_t< agg_t< sel_t< bool, agg_t<double, double>, bool, bool > >, 8>, float[16], sel_t<int, int, agg_t<char, float>, double>, char >, 0 >;
	printf("T4 -> %s : %s\n", demangle(typeid(T4).name()), demangle(typeid(get<T4, 0, 4, 2, 1 >::type).name()));
	printf("T4 -> %s : %zu\n", demangle(typeid(T4).name()), get<T4, 0, 4, 2, 1 >::index);

	using T5 = type_t< agg_t< int, int, named_t< 1, type_t< agg_t< sel_t< bool, agg_t<double, double>, bool, bool > >, 8> >, float[16], sel_t<int, int, agg_t<char, float>, double>, char >, 0 >;
	printf("T5 -> %s : %s\n", demangle(typeid(T5).name()), demangle(typeid(get<T5, 0, 4, 2, 1 >::type).name()));
	printf("T5 -> %s : %zu\n", demangle(typeid(T5).name()), get<T5, 0, 4, 2, 1 >::index);

	return 0;
}
