#include <stdio.h>
#include <aggregate.hpp>
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

	return 0;
}
