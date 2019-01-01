#include <stdio.h>
#include <type_traits>
#include <at.hpp>

int main()
{
	using namespace typu;
	printf("%d\n", std::is_same< at_type< 2, char, int, float, double >::type, float >::value);

	using Sub3 = to_types< 3, char, int, float, double >::types;
	using Sub3t = to_types< 3, char, int, float, double >::type;
	printf("%d\n", std::is_same< decltype(std::get<0>(std::declval< Sub3t >())), char >::value);
	printf("%s\n",        typeid(decltype(std::get<0>(std::declval< Sub3t >()))).name());
	printf("%s\n",               typeid(typename at_types< 0, Sub3 >::type).name());
	printf("%s\n",               typeid(typename at_types< 1, Sub3 >::type).name());
	printf("%s\n",               typeid(typename at_types< 2, Sub3 >::type).name());
	printf("%d\n", std::is_same< typename at_types< 0, Sub3 >::type, char >::value);
	printf("%d\n", std::is_same< typename at_types< 1, Sub3 >::type, int >::value);
	printf("%d\n", std::is_same< typename at_types< 2, Sub3 >::type, float >::value);
	return 0;
}
