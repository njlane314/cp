# cp

A small C++20 competitive-programming library. Each component is a
self-contained, extensionless header in the `cp` namespace.

```cpp
#include <cp/fenwick_tree>
```

The repository itself can be the include directory:

```sh
git submodule add https://github.com/njlane314/cp include/cp
```

Or install it in the usual UNIX layout:

```sh
make check
make install PREFIX="$HOME/.local"
export CPATH="$HOME/.local/include${CPATH:+:$CPATH}"
```

`PREFIX`, `DESTDIR`, and `INCLUDEDIR` are supported. There are no runtime
dependencies, generated headers, or combined catalogue header.

The public headers are `types`, `utility`, `contract`, `debug`,
`disjoint_set`, `fenwick_tree`, `segment_tree`, `modint`, and `kmp`.
