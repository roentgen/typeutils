#include <stdio.h>
#include <aggregate.hpp>

template < typename C, typename...P >
size_t foo(C c, P... p)
{
	return c;
}

struct FF {
	double d;
	float f;
};

struct F {
	char b;
	int i;
	FF d;
};

int main()
{
	using namespace typu;
	
	printf("%s\n", typeid(agg_t< int, int >::get< 0 >::type).name());
	
	printf("%zu\n", type_t< agg_t< int >, 1 >::trv());
	printf("%zu\n", type_t< agg_t< int, int >, 1 >::trv());
	printf("%zu\n", type_t< sel_t< char, int >, 1 >::trv());
	printf("%zu\n", type_t< agg_t< type_t< agg_t<int, int>, 1 >, int >, 1 >::trv());
	/* selection a select type and x  */
	printf("%zu\n", type_t< sel_t< char, sel_t< int, char > >, 1 >::trv());
	/* aggregation a selector type and x */
	printf("%zu\n", sigma_size< 1, 0, type_t< int, 1 > >::value);
	printf("%zu\n", sel_t< int >::trv());
	printf("%zu\n", sigma_size< 1, 0, sel_t_t< size_t, 1, 2, 3 > >::value);
	printf("%zu\n", type_t< agg_t< char, sel_t< int, char > , int >, 1 >::trv());

	using Simple1 = type_t< agg_t< char, char, int, double >, 1 >;
	printf("sizeof Simple1: %zu (14)\n", Simple1::trv());
	printf("char1: %zu (0)\n", Simple1::offset<0, 0>());
	printf("char2: %zu (1)\n", Simple1::offset<0, 1>());
	printf("int:   %zu (2)\n", Simple1::offset<0, 2>());
	printf("double: %zu (6)\n", Simple1::offset<0, 3>());

	using Sel1 = type_t< sel_t< int, char, bool, double >, 1 >;
	printf("sizeof Sel1: %zu (8)\n", Sel1::trv());
	printf("int: %zu (0)\n", Sel1::offset<0, 0>());
	printf("char: %zu (0)\n", Sel1::offset<0, 1>());
	printf("bool: %zu (0)\n", Sel1::offset<0, 2>());
	printf("double: %zu (0)\n", Sel1::offset<0, 3>());

	using Agg1 = type_t< agg_t< int, float, agg_t< double, float >, char >, 1 >;
	printf("sizeof Agg1: %zu (21)\n", Agg1::trv());
	printf("agg0:       %zu (0)\n", Agg1::offset<0>());
	printf("agg0:int:   %zu (0)\n", Agg1::offset<0, 0>());
	printf("agg0:float: %zu (4)\n", Agg1::offset<0, 1>());
	printf("agg1:       %zu (8)\n", Agg1::offset<0, 2>());
	printf("agg1:double %zu (8)\n", Agg1::offset<0, 2, 0>());
	printf("agg1:float  %zu (16)\n", Agg1::offset<0, 2, 1>());
	printf("agg0:char   %zu (20)\n", Agg1::offset<0, 3>());

	using Sel2 = type_t< sel_t< int, float, sel_t< double, float > >, 1 >;
	printf("sizeof Sel2: %zu (8)\n", Sel2::trv());
	printf("sel0:       %zu (0)\n", Sel2::offset<0>());
	printf("sel0:int:   %zu (0)\n", Sel2::offset<0, 0>());
	printf("sel0:float: %zu (0)\n", Sel2::offset<0, 1>());
	printf("sel1:       %zu (0)\n", Sel2::offset<0, 2>());
	printf("sel1:double %zu (0)\n", Sel2::offset<0, 2, 0>());
	printf("sel1:float  %zu (0)\n", Sel2::offset<0, 2, 1>());

	using SA1 = type_t< sel_t< int, float, agg_t< double, float > >, 1 >;
	printf("sizeof SA1: %zu (12)\n", SA1::trv());
	printf("sel0:       %zu (0)\n", SA1::offset<0>());
	printf("sel0:int:   %zu (0)\n", SA1::offset<0, 0>());
	printf("sel0:float: %zu (0)\n", SA1::offset<0, 1>());
	printf("agg0:       %zu (0)\n", SA1::offset<0, 2>());
	printf("agg0:double %zu (0)\n", SA1::offset<0, 2, 0>());
	printf("agg0:float  %zu (8)\n", SA1::offset<0, 2, 1>());

	using AS1 = type_t< agg_t< int, float, sel_t< double, float > >, 1 >;
	printf("sizeof AS1: %zu (16)\n", AS1::trv());
	printf("agg0:       %zu (0)\n", AS1::offset<0>());
	printf("agg0:int:   %zu (0)\n", AS1::offset<0, 0>());
	printf("agg0:float: %zu (4)\n", AS1::offset<0, 1>());
	printf("sel0:       %zu (8)\n", AS1::offset<0, 2>());
	printf("sel0:double %zu (8)\n", AS1::offset<0, 2, 0>());
	printf("sel0:float  %zu (8)\n", AS1::offset<0, 2, 1>());

	
	printf("bool: %zu (0)\n", Sel1::offset<0, 2>());
	printf("double: %zu (0)\n", Sel1::offset<0, 3>());
	
	using T = type_t< agg_t< char, sel_t< int, char > , sel_t< agg_t< int, int >, agg_t< double, double > >, sel_t< int > >, 0 >;
	printf("%zu\n", T::trv());
	/* offset_<T>(0, 2, 1, 1) => ç≈å„ÇÃ agg_t ÇÃìÒÇ¬ñ⁄ÇÃ double Ç‹Ç≈ÇÃ offset:
	   sizeof(T[0..0]) + 
	   sizeof(char) + sizeof(sel_t<int, char>) +
	   // sizeof(agg_t<int, int >) + 
	   sizeof(double)
	 */

	printf("offset: %zu\n", sel_t< int, sel_t< int, int > >::offset< 1, 1 >::value);
	
#if 1
	printf("%zu (5)\n", T::offset<0, 2>());
	printf("%zu (5)\n", T::offset<0, 2, 1>()); /* sel<agg<double, double>> ÇÃÇŸÇ§ÇæÇ™, sel ÇÃäJénÇ‹Ç≈ÇÃ offset Ç»ÇÃÇ≈ 5 */
	printf("%zu(13)\n", T::offset<0, 2, 1, 1>()); /* sel<agg<double, double>> ÇÃìÒÇ¬ñ⁄ÇÃ double Ç‹Ç≈ÇÃ offset Ç»ÇÃÇ≈ 13 */
	printf("%zu (5)\n", T::offset<0, 2, 0>()); /* sel<agg<int, int>> Ç≈ìØè„ */
	printf("%zu(21)\n", T::offset<0, 3>());    /* sel<agg<int,int>>,agg<double,double>> ÇÃéüÇ»ÇÃÇ≈ 21 */
#endif

	printf("%s \n", typeid(get< T, 0, 2, 1>::type).name());
	printf("size: %zu \n", get< T, 0, 2, 1>::size);
	printf("offset: %zu \n", get< T, 0, 2, 1>::offset);
	printf("%s \n", typeid(get< T, 0, 2, 1, 1>::type).name());
	printf("size: %zu \n", get< T, 0, 2, 1, 1>::size);
	printf("offset: %zu \n", get< T, 0, 2, 1, 1>::offset);
	//printf("%zu\n", T::offset<0, 2, 1, 1>());

	using Aligned0_1 = type_t< agg_t< char, char, int >, 0 >;
	/* size align Ç≈ÇÕÇ»Ç¢ÇÃÇ≈ïÅí Ç… sizeof Ç∆ìØÇ∂ */
	printf("size(align:0): %zu (8) \n", Aligned0_1::trv());
	printf("size(align:0): %zu (1) \n", get< Aligned0_1, 0, 0>::size);
	printf("size(align:0): %zu (1)\n", get< Aligned0_1, 0, 1>::size);
	printf("size(align:0): %zu (4)\n", get< Aligned0_1, 0, 2>::size);

	printf("offset(align:0): %zu (0)\n", get< Aligned0_1, 0, 0>::offset);
	printf("offset(align:0): %zu (1)\n", get< Aligned0_1, 0, 1>::offset);
	printf("offset(align:0): %zu (4)\n", get< Aligned0_1, 0, 2>::offset);

	using Aligned0_2 = type_t< agg_t< char, char, char, char, int >, 0 >;
	printf("offset(align:0): %zu (0)\n", get< Aligned0_2, 0, 0>::offset);
	printf("offset(align:0): %zu (1)\n", get< Aligned0_2, 0, 1>::offset);
	printf("offset(align:0): %zu (2)\n", get< Aligned0_2, 0, 2>::offset);
	printf("offset(align:0): %zu (3)\n", get< Aligned0_2, 0, 3>::offset);
	printf("offset(align:0): %zu (4)\n", get< Aligned0_2, 0, 4>::offset);

	using Aligned0_3 = type_t< agg_t< char, char, double, char, char, int >, 0 >;
	printf("size(align:0): %zu (24)\n", get< Aligned0_3, 0 >::size);
	printf("offset(align:0): %zu (0)\n", get< Aligned0_3, 0, 0>::offset);
	printf("offset(align:0): %zu (1)\n", get< Aligned0_3, 0, 1>::offset);
	printf("offset(align:0): %zu (8)\n", get< Aligned0_3, 0, 2>::offset);
	printf("offset(align:0): %zu (16)\n", get< Aligned0_3, 0, 3>::offset);
	printf("offset(align:0): %zu (17)\n", get< Aligned0_3, 0, 4>::offset);
	printf("offset(align:0): %zu (20)\n", get< Aligned0_3, 0, 5>::offset);

	// alighment override
	using Aligned_1_0 = type_t< agg_t< char, int, type_t< agg_t<double>, 0>, char, char, char >, 1>;
	printf("size(align:-): %zu (24)\n", get< Aligned_1_0, 0 >::size);
	printf("offset<0,0>(align:1): %zu (0)\n", get< Aligned_1_0, 0, 0>::offset);
	printf("offset<0,1>(align:1): %zu (1)\n", get< Aligned_1_0, 0, 1>::offset);
	printf("offset<0,2>(align:0): %zu (8)\n", get< Aligned_1_0, 0, 2>::offset);
	printf("size  (align:0): %zu (8)\n", get< Aligned_1_0, 0, 2>::size);
	printf("offset(align:0): %zu (8)\n", get< Aligned_1_0, 0, 2, 0, 0>::offset);
	printf("offset<0,3>(align:1): %zu (17)\n", get< Aligned_1_0, 0, 3>::offset);
	printf("offset<0,4>(align:1): %zu (18)\n", get< Aligned_1_0, 0, 4>::offset);
	printf("offset<0,5>(align:1): %zu (19)\n", get< Aligned_1_0, 0, 5>::offset);

	printf("size<0,0>(align:1): %zu (0)\n", get< Aligned_1_0, 0, 0>::size);
	printf("size<0,1>(align:1): %zu (1)\n", get< Aligned_1_0, 0, 1>::size);
	printf("size<0,2>(align:0): %zu (8)\n", get< Aligned_1_0, 0, 2>::size);

	printf("alignof:%zu\n", alignof(F));
	return 0;
}
