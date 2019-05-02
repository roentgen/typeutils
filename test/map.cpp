#include <stdio.h>
#include <aggregate.hpp>
#include <lookup.hpp>
#include <str.hpp>
#include <type_traits>
#include <typeinfo>
#include <stdint.h>
#include <cxxabi.h>

struct matrix { float m[16]; };

using namespace typu;
/* map に渡す式は M< typename >::type の形で変換後の型を導ければなんでもよいが, mapto< In >/base_mapper< Out > ヘルパを使う. */
template <> struct mapto< int > : public base_mapper< void > {} ;
template <> struct mapto< char > : public base_mapper< uint8_t > {};
template <> struct mapto< float[16] > : public base_mapper< matrix > {};
template <> struct mapto< std::array< float, 16 > > : public base_mapper< matrix > {};

template < typename In > struct mapto2 { using type = void; };

template < typename In > struct mapto3 { using type = In; };
template <> struct mapto3< agg_t< int, int > > { using type = agg_t< char, int, char >; };

int main()
{
	int status;
	using T0 = type_t< agg_t< long, int, int, char >, 0 >;
	printf("T0 -> %s\n", abi::__cxa_demangle(typeid(typename morph< T0, mapto >::mapped).name(), 0, 0, &status));

	/* 空になった集積型は消える. 
	   KNOWN ISSUE: type_t が空を収容することになるとコンパイルエラーになる */
	using T1 = type_t< agg_t< int, agg_t< int, int >, char >, 0 >;
	printf("T1 -> %s\n", abi::__cxa_demangle(typeid(typename morph< T1, mapto >::mapped).name(), 0, 0, &status));

	using T2 = type_t< agg_t< int, agg_t< int, int >, char >, 0 >;
	printf("T2 -> %s\n", abi::__cxa_demangle(typeid(typename morph< T2, mapto3 >::mapped).name(), 0, 0, &status));

	using T3 = type_t< agg_t< int, type_t< agg_t< named_t< 0, int> , int, float[16] >, 16 >, char >, 0 >;
	printf("T3 -> %s\n", abi::__cxa_demangle(typeid(typename morph< T3, mapto >::mapped).name(), 0, 0, &status));

	using T4 = type_t< agg_t< int, named_t< "matrix"_hash, type_t< agg_t< int, int, float[16] >, 16 > >, char >, 0 >;
	using S4 = typename lu< T4, "matrix"_hash >::type;
	printf("T4 %s\n", abi::__cxa_demangle(typeid(T4).name(), 0, 0, &status));
	//printf("T4 %s\n", abi::__cxa_demangle(typeid(typename morph< T4, mapto3 >::mapped).name(), 0, 0, &status));
	printf("S4 %s\n", abi::__cxa_demangle(typeid(S4).name(), 0, 0, &status));
	printf("T4 -> %s\n", abi::__cxa_demangle(typeid(typename morph< S4, mapto >::mapped).name(), 0, 0, &status));

	return 0;
}

