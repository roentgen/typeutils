#include <stdio.h>
#include <aggregate.hpp>
#include <type_traits>
#include <typeinfo>
#include <stdint.h>
#include <string.h>
#if defined(HAS_CXXABI_H)
#include <cxxabi.h>
#endif

using namespace typu;

struct mat4x4 { float m[16]; };

using CBV = type_t< agg_t< mat4x4, mat4x4 >, 16>;
using SRV = type_t< agg_t< int >, 0 >;
using Layout = type_t< agg_t< CBV, SRV >, 0 >;

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
	using T0 = type_t< agg_t< int, char >, 0ul >;
	printf("%s -> %s(flatten)\n", demangle(typeid(T0).name()), demangle(typeid(typename T0::flatten::template type< type_list<> >).name()));
	using T1 = type_t< agg_t< int, type_t< agg_t< double >, 0ul >, char >, 0ul >;
	printf("%s -> %s(flatten)\n", demangle(typeid(T1).name()), demangle(typeid(typename T1::flatten::template type< type_list<> >).name()));

	printf("%s -> %s(flatten)\n", demangle(typeid(Layout).name()), demangle(typeid(flatten_t< Layout >).name()));
	return 0;
}
