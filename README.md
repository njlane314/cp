# cp

A small C++20 competitive-programming library. Each component is a
self-contained `.hpp` header in the `cp` namespace.

```cpp
#include <cp/fenwick_tree.hpp>
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

Public indices use signed `cp::index_type`, start at zero, and all ranges are
half-open `[first, last)`. Count constructors are explicit, structures never
resize implicitly, and queries are `const` whenever they are logically read-only.

The public headers are `types.hpp`, `utility.hpp`, `contract.hpp`,
`disjoint_set.hpp`, `fenwick_tree.hpp`, `segment_tree.hpp`, `modint.hpp`,
`kmp_matcher.hpp`, `recursive.hpp`, and `coordinate_compressor.hpp`. Each header
starts with a compact synopsis and complexity reference. Developer diagnostics
live in the separate [`peek`](https://github.com/njlane314/peek) library.
