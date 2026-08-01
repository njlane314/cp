#pragma once

// <cp/recursive.hpp> — recursive lambdas without std::function
//
//   auto dfs = cp::recursive(
//       [&](auto& self, int vertex, int parent) -> void {
//           for (int next : graph[vertex])
//               if (next != parent) self(next, vertex);
//       });
//   dfs(source, -1);
//
// Construction: O(1)
// Call:         the wrapped callable's cost
//
// Recursive callables must state their return type explicitly.

#include <functional>
#include <type_traits>
#include <utility>

namespace cp {

template <class Function> class recursive {
  public:
    explicit constexpr recursive(Function function) : function_(std::move(function)) {}

    template <class... Arguments>
    constexpr auto operator()(Arguments&&... arguments) &
        noexcept(noexcept(std::invoke(function_, *this,
                                      std::forward<Arguments>(arguments)...)))
            -> std::invoke_result_t<Function&, recursive&, Arguments&&...> {
        return std::invoke(function_, *this, std::forward<Arguments>(arguments)...);
    }

    template <class... Arguments>
    constexpr auto operator()(Arguments&&... arguments) const&
        noexcept(noexcept(std::invoke(function_, *this,
                                      std::forward<Arguments>(arguments)...)))
            -> std::invoke_result_t<const Function&, const recursive&, Arguments&&...> {
        return std::invoke(function_, *this, std::forward<Arguments>(arguments)...);
    }

  private:
    [[no_unique_address]] Function function_;
};

template <class Function> recursive(Function) -> recursive<Function>;

} // namespace cp
