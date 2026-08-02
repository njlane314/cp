# libcp

A small C++20 competitive-programming library. Each component is a
self-contained, single-word header in the `cp` namespace.

The distribution is named `libcp`; its public include prefix and C++ namespace
remain `cp`.

```cpp
#include <cp/fenwick>
```

The repository itself can be the include directory:

```sh
git submodule add https://github.com/njlane314/libcp include/cp
```

Or install it in the usual UNIX layout:

```sh
make check
make install PREFIX="$HOME/.local"
export CPATH="$HOME/.local/include${CPATH:+:$CPATH}"
```

`PREFIX`, `DESTDIR`, and `INCLUDEDIR` are supported. There are no runtime
dependencies, generated headers, aliases, or combined catalogue header.

Public indices use signed `cp::index_type`, start at zero, and all ranges are
half-open `[first, last)`. Count constructors are explicit, structures never
resize implicitly, and queries are `const` whenever they are logically read-only.

The public headers are `types`, `utility`, `contract`, `disjoint`, `fenwick`,
`segment`, `modint`, `kmp`, `recursive`, and `compressor`. Each starts with a
compact synopsis and complexity reference. Its implementation lives in the
same-word `.hpp` file under `detail`, retaining reliable editor syntax
highlighting without exposing a second public spelling. Paths under `cp/detail`
are private. Developer diagnostics live in the separate
[`peek`](https://github.com/njlane314/peek) library.
