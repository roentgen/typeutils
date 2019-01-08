# typeutils

## aggregate.hpp

```c++
using namespace typu;
using T = type_t< agg_t< char, int, char, double >, 0 /* natural align */ >;

char* storage = reinterpret_cast< char* >(malloc(get<T, 0>::size));
*get<T, 0, 2>::addr(storage) = 'd';
*get<T, 0, 3>::addr(storage) = 3.141592;

```

strcuture/union like なメタタイプ.
GPU などにデータを送るとき, いちいち構造体を作らずフラットなメモリ領域をオフセットでアクセスしやすくするユーティリティ.

* struct を作らないのでメタプログラミングにフレンドリー
* alignment safe: 異なるアライン要件をネストさせても大丈夫
* アプリケーション側は cast 不要で、コンパイラによる alias 解析を妨げない
* なるべく型安全

というところを目指す.

## at.hpp

```c++
using namespace typu;

/* Index=2 を取り出す */
static_assert(std::is_same< at_type< 2, char, short, int, long >::type, int >::value, "sorry"); 

/* 先頭から 2 つ取り出す */
static_assert(std::is_same< to_types< 2, char, short, int, long >::type, std::tuple< char, short > >::value, "sorry"); 
```

tuple に包んで取り出す `to_types< N, type... >::type` と素朴な typle_list<...> を取り出す `to_types< N, type... >::types` がある.

## tuple_util.hpp

```c++
using namespace typu;

auto huge = std::make_tuple('t', 'h', 'i', 's', ' ', 'i', 's', ' ', 'g', 'o', 'd', 'd', 'a', 'm', 'n', ' ', 'h', 'u', 'g', 'e', ' ', 'o', 'n', 'e');

auto nice = selector(huge, iv<16>('n'), iv<17>('i'), iv<18>('c'));
```

調子に乗って tuple をでかくしすぎてしまったときに狙ったところだけ置き換える `selector()` を提供.
constexpr なのでメタプロで絶賛再帰中の Acc でやらかしたときも使える.
