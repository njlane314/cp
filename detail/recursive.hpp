#pragma once

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
