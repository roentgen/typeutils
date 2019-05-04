#include <str.hpp>
#include <typeinfo>
#include <stdio.h>

using namespace typu;

int main()
{
#if !defined(_MSC_VER)
	/*
	  MSVC のために DECO マクロを用意したが C3477 の制約のために 
	  typeid() 内で lambda を宣言できず, マクロ化も不可能だった.
	 */
	printf(": %s\n", typeid(DECO("")).name());
	printf("a: %s\n", typeid(DECO("a")).name());
	printf("abc: %s\n", typeid(DECO("abc")).name());
	printf("abc: %s\n", typeid(DECO(u8"abc")).name()); // u8
	printf("abc: %s\n", typeid(DECO(U"abc")).name()); // u32
#endif
	printf("hash abc: %zu %s\n", "abc"_hash, "abc"_hash == 193485963 ? "true" : "false");
	printf("hash ABC: %zu %s\n", "ABC"_hash, "ABC"_hash == 193450027 ? "true" : "false");
	printf("hash x0000: %zu %s\n", "x0000"_hash, "x0000"_hash == 210731639133 ? "true" : "false");
	printf("hash X0000: %zu %s\n", "X0000"_hash, "X0000"_hash == 210693689661 ? "true" : "false");
	// only alphabets, digits, and '_'. others will fail to an assertion
	//printf("hash [abc]: %zu\n", "[abc]"_hash);

	// conflicts
	printf("hash FYFYFYEzFYFYFYEzFYEzFYFYFYFYFY == hash EzEzEzEzEzEzEzEzEzEzEzEzEzEzEz: conflicts:%s\n", "FYFYFYEzFYFYFYEzFYEzFYFYFYFYFY"_hash == "EzEzEzEzEzEzEzEzEzEzEzEzEzEzEz"_hash ? "true" : "false");
	return 0;
}
