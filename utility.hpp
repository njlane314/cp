#pragma once

// <cp/utility.hpp> — small value-update helpers
//
//   cp::chmin(value, candidate);
//   cp::chmax(value, candidate);
//
// chmin/chmax: O(1)

namespace cp {

template <class T> constexpr bool chmin(T& value, const T& candidate) {
    if (!(candidate < value)) return false;
    value = candidate;
    return true;
}

template <class T> constexpr bool chmax(T& value, const T& candidate) {
    if (!(value < candidate)) return false;
    value = candidate;
    return true;
}

} // namespace cp
