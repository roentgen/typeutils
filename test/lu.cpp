/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#include <stdio.h>
#include <aggregate.hpp>
#include <lookup.hpp>

int main()
{
	using namespace typu;

	using Simple1 = type_t< agg_t< char, char, named_t<0xdeadbeef, int>, double >, 1 >;
	printf("%s\n", typeid(Simple1::get< 0, 0 >::type).name());
	printf("%s\n", typeid(Simple1::get< 0 >::type).name());
	printf("%s\n", typeid(Simple1::get< 0, 0, 0 >::type).name());
	printf("%s\n", typeid(Simple1::get< 0, 0 >::type::get<0>::type).name());
	printf("get %d\n", has_get< Simple1 >::value);
	printf("get %d\n", has_get< Simple1::get< 0, 0 >::type >::value);
	printf("int:   %zu (2)\n", lu< Simple1, 0xdeadbeef >::offset);

	auto cac0 = test_or_descend_<0xdeadbeef, Simple1::get< 0, 0 >::type, 0 >();
	auto cac1 = test_or_descend_<0xdeadbeef, Simple1::get< 0, 0 >::type, 1 >();
	auto cac2 = test_or_descend_<0xdeadbeef, Simple1::get< 0, 0 >::type, 2 >();
	static_assert(std::is_same< decltype(cac0), std::index_sequence<> >::value, "must empty");
	static_assert(std::is_same< decltype(cac1), std::index_sequence<> >::value, "must empty");
	static_assert(std::is_same< decltype(cac2), std::index_sequence<2> >::value, "must not empty");

	using Simple2 = type_t< agg_t< char, char, int, double, named_t< 0xdeadbeef, agg_t< int, int > > >, 1 >;
	printf("agg:   %zu (14)\n", lu< Simple2, 0xdeadbeef >::offset);
	printf("%s\n", typeid(typename lu< Simple2, 0xdeadbeef >::type).name());

	return 0;
}

