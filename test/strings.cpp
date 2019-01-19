#include <str.hpp>
#include <typeinfo>
#include <stdio.h>

using namespace typu;

int main()
{
	printf(": %s\n", typeid(DECO("")).name());
	printf("a: %s\n", typeid(DECO("a")).name());
	printf("abc: %s\n", typeid(DECO("abc")).name());
	printf("abc: %s\n", typeid(DECO(u8"abc")).name()); // u8
	printf("abc: %s\n", typeid(DECO(U"abc")).name()); // u32
	printf("hash abc: %zu %s\n", "abc"_hash, "abc"_hash == 193485963 ? "true" : "false");
	printf("hash ABC: %zu %s\n", "ABC"_hash, "ABC"_hash == 193450027 ? "true" : "false");
	printf("hash x0000: %zu %s\n", "x0000"_hash, "x0000"_hash == 210731639133 ? "true" : "false");
	printf("hash X0000: %zu %s\n", "X0000"_hash, "X0000"_hash == 210693689661 ? "true" : "false");
	// conflicts
	printf("hash FYFYFYEzFYFYFYEzFYEzFYFYFYFYFY == hash EzEzEzEzEzEzEzEzEzEzEzEzEzEzEz: conflicts:%s\n", "FYFYFYEzFYFYFYEzFYEzFYFYFYFYFY"_hash == "EzEzEzEzEzEzEzEzEzEzEzEzEzEzEz"_hash ? "true" : "false");
	return 0;
}
