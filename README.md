# typeutils

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

structure/union like なメタタイプ.
GPU などにデータを送るとき, いちいち構造体を作らずフラットなメモリ領域をオフセットでアクセスしやすくするユーティリティ.

* struct を作らないのでメタプログラミングにフレンドリー
* alignment safe: 異なるアライン要件をネストさせても大丈夫
* map を使って同相の型を新しく生成できる
* アプリケーション側は cast 不要で、コンパイラによる alias 解析を妨げない
* なるべく型安全

というところを目指す.

### type_t<T, Align>

ただ一つの型を収容し, 収容する型に対してアラインメント制限を明示する. Align = 0 のとき自然なアラインメントになる.
`type_t` は, `agg_t`, `sel_t` を収容する.  
`type_t` はネストしてもよく, より厳しい/緩いアラインメント制限を加えることができる. このため, type_t の前後にパディングを伴うことがある.

### 集約型 agg_t<Ts...>/sel_t<Ts...>

任意個の型を収容する集約型. agg_t は struct に, sel_t は union に対応する. コンストラクタ, デストラクタ, 仮想関数を持つことはできない.

### ノーテーション named_t<Name, T>

任意の整数 Named を T に与える. `lookup` で利用される.  
集約型ではないため自身のインデックスを持たず, offset の計算にも影響しない. 後述の map/fold の対象にはなる.

### get 

get インターフェイスは収容型にインデックスアクセスを行う.  
以下の例では type は `<0, 4>` の型になる. `0` は `type_t` が直接収容する唯一の `agg_t` を示すインデックスである.

```c++
using T = type_t< agg_t< int, int, int, int, float, double >, 0 >;
using S = typename get<T, 0, 4 >::type; /* float */
```

ネストした場合にはインデックスの次元が増える.
また, get した型への `offset`, 指定アドレスからのアドレスの計算 `addr`, リーフインデックス `index` がある.

```c++
size_t ofs = get< T0, 0, 1, 0, 0 >::offset;
*get<T0, 0, 1, 0, 0>::addr(base) = 0xdeadbeef;
```

`get<T, Index...>::index` は `T` の記憶領域を持つ型のうち, `Index...` で示される型までのインデックスになる.  
(TBD: 間に `sel_t` がある場合は sel_t のうち最もリーフ数の大きい型と仮定する)

```c++
using T1 = type_t< agg_t< int, int, agg_t< bool, bool, bool >, float[16], agg_t<int, int, float, double>, char >, 0 >;
size_t idx = get<T2, 0, 4, 2 >::index; /* float's index = 8 */
```

### map<Fun...>

map は型の変換を行う. 以下の例では float[16], float[4] を対応する代数型に変換する.

```c++
using namespace typu;
template <> struct typu::mapto< float[16] > : public base_mapper< matrix44_t > {};
template <> struct typu::mapto< float[4] > : public base_mapper< vector4_t > {};

using T = type_t< agg_t< float[16], float[4], float[16] >, 16 >;
using S = typename morph< T, mapto >::mapped; /* type_t< agg_t< matrix44_t, vector4_t, matrix44_t >, 16 > */
```

`fun(int)->void` を map して型を削除することもできる.　削除する前の rawtype を得ることもできる.  
この結果, 記憶領域を持たなくなった型は削除される.

```c++
using namespace typu;
template <> struct typu::mapto< int > : public base_mapper< void > {};

using T = type_t< agg_t< int, named_t< "int"_hash, type_t< agg_t< int, named_t< 0, int > >, 16 > >, char >, 0 >;
using S = typename morph< T, mapto >::mapped; /* type_t< char, 0 > */
```

### fold<T, Acc, Acc0, Fun... >

(TBD)

fold は `T` の収容する型に対して, `Fun...` を実行する. 引数は `Acc` 型の初期値 `Acc0` と, `T` の収容型である.
次の例では `int` の数を数える.

```c++
using namespace typu;
template < typename Acc, Acc Acc0, typename In >
struct fold_fun { static const Acc value = Acc0; };
template <typename Acc, Acc Acc0 >
struct fold_fun< Acc, Acc0, int > { static const Acc value = Acc0 + 1; };

using T = type_t< agg_t< int, agg_t< int >, agg_t< float[16] >, char >, 0 >;
auto n = fold< T, int, 0, fold_fun >::value; // 2
```

`sel_t` の全ての収容型にも再帰的に適用される.


## lookup.hpp

```c++
using namespace typu;
using T = type_t< agg_t< char, int, char, named_t< 0xdeadbeef, double >, 0 /* natural align */ >;

static_assert(std::is_same< double, typename lu<T, 0xdeadbeef>::type >::value, "found type must be double");
printf("offset: %zu\n", lu<T, 0xdeadbeef>::offset);
```

`named_t<Name, T>` 任意の一意なキー値を使って, 型に名前をつけることができる.
名前を介してのアクセスには lu<T, symbol > を使って get と同様コンパイル時定数として行える.


## at.hpp

```c++
using namespace typu;

/* Index=2 を取り出す */
static_assert(std::is_same< at_type< 2, char, short, int, long >::type, int >::value, "sorry"); 

/* 先頭から 2 つ取り出す */
static_assert(std::is_same< to_types< 2, char, short, int, long >::type, std::tuple< char, short > >::value, "sorry"); 
```

tuple に包んで取り出す `to_types< N, type... >::type` と素朴な typle_list<...> を取り出す `to_types< N, type... >::types` がある.
