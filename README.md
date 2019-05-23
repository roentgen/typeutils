# typeutils

[日本語版](README.ja.md)

## aggregate.hpp

```c++
using namespace typu;
using T = type_t< agg_t< char, int, char, double >, 0 /* natural align */ >;

char* storage = reinterpret_cast< char* >(malloc(get<T, 0>::size));
*get<T, 0, 2>::addr(storage) = 'd';
*get<T, 0, 3>::addr(storage) = 3.141592;

/* convert double -> float */
template <> struct mapto< double > : public base_mapper< float >{};
using S = typename morph< T, mapto >::mapped;
```

A computable metatype: builidng and manipurating types like modifing struct/union.

GPU and other dedicated devices require data and its layout.
This helps generating types which dedicate device requires raw-data and its descriptors from a CPU-friendly type in compile-time.

* non-struct: compositing types provide meta-programmable type
* alignment-safe: type_t(s) are compositable with strong/week alignment requirements
* morph: generates homeomorphic type using map
* typesafe: may avoid to prevent compiler's analyzing alias due to the application be less typecast

clang5.0 or later. or MSVC2017.


### type_t<T, Align>

Contains a single type, and provide requirement of alignment. Align=0 means a natural alignment.  
`type_t` will often contains `agg_t` or `sel_t`.
Nesting `type_t` provides more strong/week alignment than container. This may put a padding before/after of containee.

### Aggregation Type agg_t<Ts...>/sel_t<Ts...>

Contains any types.  
`agg_t`/`sel_t` corresponds to `struct`/`union`.
But these container (not containee) shall not be have dedicate constructors, destructors and virtual functions.
For containee, no dedicate restructions but it should not be `void`.

### Notation named_t<Name, T>

Gives a symbol, `Name`, as single containee `T`. The symbol will be used in `lookup`.
It will have no effect for calculation of `offset` that a type does not have its index since it's not aggregation type, but will be a participant for map/fold's.

### get<T, Index...>

Accesses member-type by indices.  
In the following example, `type` will be `float` type indexed by `<0, 4>`. the first `0` means the directly containee `agg_t`.

```c++
using T = type_t< agg_t< int, int, int, int, float, double >, 0 >;
using S = typename get<T, 0, 4 >::type; /* float */
```

Nesting types requires higher dimension indices.  
Also `get` has `offset`, `addr` from effective-address and leaf-index, `index` for the indexed type.

```c++
size_t ofs = get< T0, 0, 1, 0, 0 >::offset;
*get<T0, 0, 1, 0, 0>::addr(base) = 0xdeadbeef;
```

`get<T, Index...>::index` will be replaced to the indexed type by `Index...` where has its storage on 'T'.
(TBD: When a `sel_t`'s member is in a type, `T`, will be assumed or selected the largest type of members)

```c++
using T1 = type_t< agg_t< int, int, agg_t< bool, bool, bool >, float[16], agg_t<int, int, float, double>, char >, 0 >;
size_t idx = get<T2, 0, 4, 2 >::index; /* float's index = 8 */
```

### morph<T, Fun...>

Gives the new type as map.
In the following example, `morph` will convert `float[16]` and `float[4]` to correspond algebra type, matrix and vector. `mapped` will be mapped type.  
Function's argument will be matched node types like as `agg_t`, `sel_t` or `type_t`.

```c++
using namespace typu;
template <> struct typu::mapto< float[16] > : public base_mapper< matrix44_t > {};
template <> struct typu::mapto< float[4] > : public base_mapper< vector4_t > {};

using T = type_t< agg_t< float[16], float[4], float[16] >, 16 >;
using S = typename morph< T, mapto >::mapped; /* type_t< agg_t< matrix44_t, vector4_t, matrix44_t >, 16 > */
```

Also `morph` is able to remove types by mapping them `void`. In the following example, it `morph`s `int` to `void` via a function `fun(int)->void`. A function is expressed in template's full specialization.
Then `void` and types has no its storage will be sanitized.  
If you need a full type you can access it with `mapped_raw`.

```c++
using namespace typu;
template <> struct typu::mapto< int > : public base_mapper< void > {};

using T = type_t< agg_t< int, named_t< "int"_hash, type_t< agg_t< int, named_t< 0, int > >, 16 > >, char >, 0 >;
using S = typename morph< T, mapto >::mapped; /* type_t< char, 0 > */
```

### fold<T, Acc, Acc0, Fun... >

(TBD)

Applies `Fun...` to `T`'s all containees and gives a value typed as `Acc`.
A function's argument will be matched node types like `agg_t`, `sel_t` or `type_t`, morph as well.  
Next example shows counting `int` of `T`.  

```c++
using namespace typu;
template < typename Acc, Acc Acc0, typename In >
struct fold_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_fun< Acc, Acc0, int > { static const Acc value = Acc0 + 1; };

using T = type_t< agg_t< int, agg_t< int >, agg_t< float[16] >, char >, 0 >;
auto n = fold< T, int, 0, fold_fun >::value; // 2
```

Functions will be applied to `sel_t`'s all containees.


## lookup.hpp

```c++
using namespace typu;
using T = type_t< agg_t< char, int, char, named_t< 0xdeadbeef, double >, 0 /* natural align */ >;

static_assert(std::is_same< double, typename lu<T, 0xdeadbeef>::type >::value, "found type must be double");
printf("offset: %zu\n", lu<T, 0xdeadbeef>::offset);
```

`named_t<Name, T>` names `T` as unique symbol, `Name`.
Looking the type up via `lu<T, Symbol>` is done at compile-time as well `get` or other functions do.


## at.hpp

This header has utility to manipurate variadic template parameter.

```c++
using namespace typu;

/* get a type Index=2 */
static_assert(std::is_same< at_type< 2, char, short, int, long >::type, int >::value, "sorry"); 

/* get 2 types from a head */
static_assert(std::is_same< to_types< 2, char, short, int, long >::type, std::tuple< char, short > >::value, "sorry"); 
```

`to_types<N, T...>` has two variations. One is `to_types<>::type`, which uses `std::tuple`. Another is `to_types<>::types` which uses simple `type_list<...>`.
