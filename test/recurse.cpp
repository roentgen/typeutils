/**
 * (C) roentgen 
 * Released under the MIT license.
 */
#include <stdio.h>
#include <aggregate.hpp>

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
	printf("%zu\n", type_t< agg_t< char, sel_t< int, char > , int >, 1 >::trv());

	using Simple1 = type_t< agg_t< char, char, int, double >, 1 >;
	printf("sizeof Simple1: %zu (14)\n", Simple1::trv());
	printf("char1: %zu (0)\n", Simple1::offset_from<0, 0>());
	printf("char2: %zu (1)\n", Simple1::offset_from<0, 1>());
	printf("int:   %zu (2)\n", Simple1::offset_from<0, 2>());
	printf("double: %zu (6)\n", Simple1::offset_from<0, 3>());

	using Sel1 = type_t< sel_t< int, char, bool, double >, 1 >;
	printf("sizeof Sel1: %zu (8)\n", Sel1::trv());
	printf("int: %zu (0)\n", Sel1::offset_from<0, 0>());
	printf("char: %zu (0)\n", Sel1::offset_from<0, 1>());
	printf("bool: %zu (0)\n", Sel1::offset_from<0, 2>());
	printf("double: %zu (0)\n", Sel1::offset_from<0, 3>());

	using Agg1 = type_t< agg_t< int, float, agg_t< double, float >, char >, 1 >;
	printf("sizeof Agg1: %zu (21)\n", Agg1::trv());
	printf("agg0:       %zu (0)\n", Agg1::offset_from<0>());
	printf("agg0:int:   %zu (0)\n", Agg1::offset_from<0, 0>());
	printf("agg0:float: %zu (4)\n", Agg1::offset_from<0, 1>());
	printf("agg1:       %zu (8)\n", Agg1::offset_from<0, 2>());
	printf("agg1:double %zu (8)\n", Agg1::offset_from<0, 2, 0>());
	printf("agg1:float  %zu (16)\n", Agg1::offset_from<0, 2, 1>());
	printf("agg0:char   %zu (20)\n", Agg1::offset_from<0, 3>());

	using Sel2 = type_t< sel_t< int, float, sel_t< double, float > >, 1 >;
	printf("sizeof Sel2: %zu (8)\n", Sel2::trv());
	printf("sel0:       %zu (0)\n", Sel2::offset_from<0>());
	printf("sel0:int:   %zu (0)\n", Sel2::offset_from<0, 0>());
	printf("sel0:float: %zu (0)\n", Sel2::offset_from<0, 1>());
	printf("sel1:       %zu (0)\n", Sel2::offset_from<0, 2>());
	printf("sel1:double %zu (0)\n", Sel2::offset_from<0, 2, 0>());
	printf("sel1:float  %zu (0)\n", Sel2::offset_from<0, 2, 1>());

	using SA1 = type_t< sel_t< int, float, agg_t< double, float > >, 1 >;
	printf("sizeof SA1: %zu (12)\n", SA1::trv());
	printf("sel0:       %zu (0)\n", SA1::offset_from<0>());
	printf("sel0:int:   %zu (0)\n", SA1::offset_from<0, 0>());
	printf("sel0:float: %zu (0)\n", SA1::offset_from<0, 1>());
	printf("agg0:       %zu (0)\n", SA1::offset_from<0, 2>());
	printf("agg0:double %zu (0)\n", SA1::offset_from<0, 2, 0>());
	printf("agg0:float  %zu (8)\n", SA1::offset_from<0, 2, 1>());

	using AS1 = type_t< agg_t< int, float, sel_t< double, float > >, 1 >;
	printf("sizeof AS1: %zu (16)\n", AS1::trv());
	printf("agg0:       %zu (0)\n", AS1::offset_from<0>());
	printf("agg0:int:   %zu (0)\n", AS1::offset_from<0, 0>());
	printf("agg0:float: %zu (4)\n", AS1::offset_from<0, 1>());
	printf("sel0:       %zu (8)\n", AS1::offset_from<0, 2>());
	printf("sel0:double %zu (8)\n", AS1::offset_from<0, 2, 0>());
	printf("sel0:float  %zu (8)\n", AS1::offset_from<0, 2, 1>());

	
	printf("bool: %zu (0)\n", Sel1::offset_from<0, 2>());
	printf("double: %zu (0)\n", Sel1::offset_from<0, 3>());
	
	using T = type_t< agg_t< char, sel_t< int, char > , sel_t< agg_t< int, int >, agg_t< double, double > >, sel_t< int > >, 0 >;
	printf("%zu\n", T::trv());
	/* offset_<T>(0, 2, 1, 1) => 最後の agg_t の二つ目の double までの offset:
	   sizeof(T[0..0]) + 
	   sizeof(char) + sizeof(sel_t<int, char>) +
	   // sizeof(agg_t<int, int >) + 
	   sizeof(double)
	 */

	printf("offset: %zu\n", sel_t< int, sel_t< int, int > >::offset< 1, 1 >::value);
	
#if 1
	printf("%zu (5)\n", T::offset_from<0, 2>());
	printf("%zu (5)\n", T::offset_from<0, 2, 1>()); /* sel<agg<double, double>> のほうだが, sel の開始までの offset なので 5 */
	printf("%zu(13)\n", T::offset_from<0, 2, 1, 1>()); /* sel<agg<double, double>> の二つ目の double までの offset なので 13 */
	printf("%zu (5)\n", T::offset_from<0, 2, 0>()); /* sel<agg<int, int>> で同上 */
	printf("%zu(21)\n", T::offset_from<0, 3>());    /* sel<agg<int,int>>,agg<double,double>> の次なので 21 */
#endif

	printf("%s \n", typeid(get< T, 0, 2, 1>::type).name());
	printf("size: %zu \n", get< T, 0, 2, 1>::size);
	printf("offset: %zu \n", get< T, 0, 2, 1>::offset);
	printf("%s \n", typeid(get< T, 0, 2, 1, 1>::type).name());
	printf("size: %zu \n", get< T, 0, 2, 1, 1>::size);
	printf("offset: %zu \n", get< T, 0, 2, 1, 1>::offset);
	//printf("%zu\n", T::offset<0, 2, 1, 1>());

	using Aligned0_1 = type_t< agg_t< char, char, int >, 0 >;
	/* size align ではないので普通に sizeof と同じ */
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

	using Sub = agg_t< char, char, char >;
	using Sub2 = agg_t< char, Sub, char >;
	using Aligned0_4 = type_t< agg_t< char, char, char, Sub, Sub, Sub2, int, char >, 0 >;
	printf("size(align:0): %zu (21)\n", get< Aligned0_4, 0 >::size);
	printf("offset: %zu (0) <0,0>\n", get< Aligned0_4, 0, 0>::offset);
	printf("offset: %zu (1) <0,1>\n", get< Aligned0_4, 0, 1>::offset);
	printf("offset: %zu (2) <0,2>\n", get< Aligned0_4, 0, 2>::offset);
	printf("offset: %zu (3) <0,3>\n", get< Aligned0_4, 0, 3>::offset);
	printf("offset: %zu (3) <0,3,0>\n", get< Aligned0_4, 0, 3, 0>::offset);
	printf("offset: %zu (4) <0,3,1>\n", get< Aligned0_4, 0, 3, 1>::offset);
	printf("offset: %zu (5) <0,3,2>\n", get< Aligned0_4, 0, 3, 2>::offset);
	printf("offset: %zu (6) <0,4,0>\n", get< Aligned0_4, 0, 4, 0>::offset);
	printf("offset: %zu (7) <0,4,1>\n", get< Aligned0_4, 0, 4, 1>::offset);
	printf("offset: %zu (8) <0,4,2>\n", get< Aligned0_4, 0, 4, 2>::offset);
	printf("offset: %zu (9) <0,5,0>\n", get< Aligned0_4, 0, 5, 0>::offset);
	printf("offset: %zu (10) <0,5,1>\n", get< Aligned0_4, 0, 5, 1>::offset);
	printf("offset: %zu (10) <0,5,1,0>\n", get< Aligned0_4, 0, 5, 1, 0>::offset);
	printf("offset: %zu (11) <0,5,1,1>\n", get< Aligned0_4, 0, 5, 1, 1>::offset);
	printf("offset: %zu (12) <0,5,1,2>\n", get< Aligned0_4, 0, 5, 1, 2>::offset);
	printf("offset: %zu (13)<0,5,2>\n", get< Aligned0_4, 0, 5, 2>::offset);
	printf("offset: %zu (16)<0,6>\n", get< Aligned0_4, 0, 6>::offset);
	printf("offset: %zu (20)<0,7>\n", get< Aligned0_4, 0, 7>::offset);

	using T0 = type_t< agg_t< char, type_t< agg_t< int >, 0> >, 1 >;
	printf("offset: %zu <0,1>    (1) align:1\n", get< T0, 0, 1 >::offset);
	printf("offset: %zu <0,1,0>  (4) align:1->0\n", get< T0, 0, 1, 0 >::offset);
	printf("offset: %zu <0,1,0,0>(4) align:0\n", get< T0, 0, 1, 0, 0 >::offset);

	void* storage = malloc(get<T0, 0>::size);
	memset(storage, 0xff, get<T0, 0>::size);
	*get<T0, 0, 0>::addr(storage) = 'a';
	*get<T0, 0, 1, 0, 0>::addr(storage) = 0xdeadbeef;
	for (int i = 0; i < get<T0, 0>::size; i ++) {
		printf("%02x ", *(reinterpret_cast< char* >(storage) + i) & 0xff);
	}
	printf ("\n");
	free(storage);
	
	using SubT = type_t < agg_t< char, char, char >, 0 >;
	using SubT2 = type_t < agg_t< char, SubT, char >, 0 >;
	using Aligned0_5 = type_t< agg_t< char, char, char, SubT, SubT, SubT2, int, char >, 0 >;
	printf("size(align:0): %zu (21)\n", get< Aligned0_5, 0 >::size);
	printf("offset: %zu (0) <0,0>\n", get< Aligned0_5, 0, 0>::offset);
	printf("offset: %zu (1) <0,1>\n", get< Aligned0_5, 0, 1>::offset);
	printf("offset: %zu (2) <0,2>\n", get< Aligned0_5, 0, 2>::offset);
	printf("offset: %zu (3) <0,3>\n", get< Aligned0_5, 0, 3>::offset);
	printf("offset: %zu (3) <0,3,0,0>\n", get< Aligned0_5, 0, 3, 0, 0>::offset);
	printf("offset: %zu (4) <0,3,0,1>\n", get< Aligned0_5, 0, 3, 0, 1>::offset);
	printf("offset: %zu (5) <0,3,0,2>\n", get< Aligned0_5, 0, 3, 0, 2>::offset);
	printf("offset: %zu (6) <0,4,0,0>\n", get< Aligned0_5, 0, 4, 0, 0>::offset);
	printf("offset: %zu (7) <0,4,0,1>\n", get< Aligned0_5, 0, 4, 0, 1>::offset);
	printf("offset: %zu (8) <0,4,0,2>\n", get< Aligned0_5, 0, 4, 0, 2>::offset);
	printf("offset: %zu (9) <0,5,0,0>\n",    get< Aligned0_5, 0, 5, 0, 0>::offset);
	printf("offset: %zu (10) <0,5,0,1>\n",   get< Aligned0_5, 0, 5, 0, 1>::offset);
	printf("offset: %zu (10) <0,5,0,1,0,0>\n", get< Aligned0_5, 0, 5, 0, 1, 0, 0>::offset);
	printf("offset: %zu (11) <0,5,0,1,0,1>\n", get< Aligned0_5, 0, 5, 0, 1, 0, 1>::offset);
	printf("offset: %zu (12) <0,5,0,1,0,2>\n", get< Aligned0_5, 0, 5, 0, 1, 0, 2>::offset);
	printf("offset: %zu (13)<0,5,2>\n",    get< Aligned0_5, 0, 5, 0, 2>::offset);
	printf("offset: %zu (16)<0,6>\n", get< Aligned0_5, 0, 6>::offset);
	printf("offset: %zu (20)<0,7>\n", get< Aligned0_5, 0, 7>::offset);

	// alighment override
	using Aligned_1_0 = type_t< agg_t< char, int, type_t< agg_t<double>, 0>, char, char, char >, 1>;
	/*     0 1    5   8      F 
          |-+----+---+--------|
	  +0  |1|   4|pad|       8|
           type_t^   ^agg_t
      +10 |1|1|1|
	 */
	printf("size(align:-): %zu (19)\n", get< Aligned_1_0, 0 >::size);
	printf("offset<0,0>(align:1): %zu (0)\n", get< Aligned_1_0, 0, 0>::offset);
	printf("offset<0,1>(align:1): %zu (1)\n", get< Aligned_1_0, 0, 1>::offset);
	/* align=0 の type_t は上位 enclosure の placement align に従う */
	printf("offset<0,2>(align:0): %zu (5)\n", get< Aligned_1_0, 0, 2>::offset);
	printf("size: %zu (8)\n", get< Aligned_1_0, 0, 2>::size);
#if 0
	printf("size: %zu (8) placement-align:1 type_t-align:%zu real-align:%zu\n", get< Aligned_1_0, 0, 2>::type::placement<1>(5),
		   get< Aligned_1_0, 0, 2>::type::align,
		   alignof_< get< Aligned_1_0, 0, 2, 0>::type >());
#endif
	/* agg_t の alignment は type_t に従う */
	printf("offset<0,2,0>(align:0): %zu (8)\n", get< Aligned_1_0, 0, 2, 0>::offset);
	printf("offset<0,2,0,0>(align:0): %zu (8)\n", get< Aligned_1_0, 0, 2, 0, 0>::offset);
	//printf("align (align:0): %zu (8)\n", get< Aligned_1_0, 0, 2>::type::alignof_()); // debug
	printf("offset<0,3>(align:1): %zu (16)\n", get< Aligned_1_0, 0, 3>::offset);
	printf("offset<0,4>(align:1): %zu (17)\n", get< Aligned_1_0, 0, 4>::offset);
	printf("offset<0,5>(align:1): %zu (18)\n", get< Aligned_1_0, 0, 5>::offset);

	/* 前方,後方に padding が入る(どちらも type_t の外に padding がある) */
	using Aligned_0_1 = type_t< agg_t< char, int, char, type_t< agg_t<double>, 2>, int, char, char >, 0>;
	/*     0    4    8  |A    F
          |-+--+----+-+-+------|
	  +0  |1|pa|   4|1|p|   6/8|
                        ^type_t/agg_t(align=2)
          2/8|pa|   4|1|1|
      +10 |--+--+----+-+-+
           0 |2 |4   |8|9|
            ~^  ^int
	 */
	printf("size(align:-): %zu (26)\n", get< Aligned_0_1, 0 >::size);
	printf("offset<0,0>(align:0): %zu (0)\n", get< Aligned_0_1, 0, 0>::offset);
	printf("offset<0,1>(align:0): %zu (4)\n", get< Aligned_0_1, 0, 1>::offset);
	printf("offset<0,2>(align:0): %zu (8)\n", get< Aligned_0_1, 0, 2>::offset);
	printf("offset<0,3>(align:1): %zu (10)\n", get< Aligned_0_1, 0, 3>::offset);
	printf("size  (align:1): %zu (8)\n", get< Aligned_0_1, 0, 3>::size);
	printf("offset(align:1): %zu (10)\n", get< Aligned_0_1, 0, 3, 0, 0>::offset);
	printf("type_t align<0,3> (align:0): %zu (2)\n", get< Aligned_0_1, 0, 3>::type::alignof_()); // debug
	printf("real align<0,3> (align:0): %zu (8)\n", typu::alignof_< get< Aligned_0_1, 0, 3>::type::sub >()); // debug
	printf("size  (align:1): %zu (8)\n", get< Aligned_0_1, 0, 3>::type::trv<1>(10)); // debug
	printf("size  (align:0): %zu (8)\n", get< Aligned_0_1, 0, 3>::size); // debug
	printf("size  (align:0): %zu (8) padded\n", get< Aligned_0_1, 0, 3>::type::placement<0>(10)); // debug
	printf("offset<0,4>(align:0): %zu (20)\n", get< Aligned_0_1, 0, 4>::offset);
	//printf("align (align:0): %zu (4)\n", Aligned_0_1::alignof_()); // debug
	printf("offset<0,5>(align:0): %zu (24)\n", get< Aligned_0_1, 0, 5>::offset);
	printf("offset<0,6>(align:0): %zu (25)\n", get< Aligned_0_1, 0, 6>::offset);

	
	//printf("align  (align:0): %zu (20) padded\n", get< Aligned_0_1, 0, 4>::type::trv<0>(20)); // debug
	return 0;
}
