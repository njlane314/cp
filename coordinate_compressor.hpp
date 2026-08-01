#pragma once

// <cp/coordinate_compressor.hpp> — sorted ranks for arbitrary values
//
//   cp::coordinate_compressor coordinates(values);
//   auto compressed = coordinates.rank(value);
//   const auto original = coordinates.value(compressed);
//
// Indices: zero-based
// Build:   O(n log n)
// rank:    O(log n)
// value:   O(1)
//
// Keywords: coordinate compression, rank, discretization
// Precondition: rank(x) requires x to occur in the construction range.
// Round trip: value(rank(x)) is comparator-equivalent to x.

#include "cp/contract.hpp"
#include "cp/types.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace cp {

template <class T, class Compare = std::less<T>> class coordinate_compressor {
  public:
    using value_type = T;
    using compare_type = Compare;

    coordinate_compressor() = default;

    template <std::ranges::input_range Range>
        requires std::convertible_to<std::ranges::range_reference_t<Range>, T>
    explicit coordinate_compressor(Range&& values, Compare compare = {})
        : compare_(std::move(compare)) {
        if constexpr (std::ranges::sized_range<Range>) {
            const auto count = std::ranges::size(values);
            if (std::cmp_greater(count, std::numeric_limits<index_type>::max()))
                detail::contract_fail("size() <= max(index_type)",
                                      "coordinate_compressor: input is too large");
            values_.reserve(static_cast<std::size_t>(count));
        }
        for (auto&& value : values)
            values_.push_back(static_cast<T>(std::forward<decltype(value)>(value)));

        std::sort(values_.begin(), values_.end(), compare_);
        values_.erase(std::unique(values_.begin(), values_.end(),
                                  [this](const T& first, const T& second) {
                                      return equivalent(first, second);
                                  }),
                      values_.end());
        if (values_.size() > static_cast<std::size_t>(
                                 std::numeric_limits<index_type>::max()))
            detail::contract_fail("size() <= max(index_type)",
                                  "coordinate_compressor: input is too large");
    }

    [[nodiscard]] index_type size() const noexcept {
        return static_cast<index_type>(values_.size());
    }
    [[nodiscard]] bool empty() const noexcept { return values_.empty(); }

    [[nodiscard]] index_type rank(const T& value) const {
        const auto iterator = lower_bound(value);
        CP_EXPECT(iterator != values_.end() && equivalent(*iterator, value),
                  "coordinate_compressor::rank: value is absent");
        return static_cast<index_type>(iterator - values_.begin());
    }

    [[nodiscard]] typename std::vector<T>::const_reference value(index_type position) const {
        CP_EXPECT(0 <= position && position < size(),
                  "coordinate_compressor::value: invalid position");
        return values_[static_cast<std::size_t>(position)];
    }

  private:
    [[no_unique_address]] Compare compare_;
    std::vector<T> values_;

    [[nodiscard]] bool equivalent(const T& first, const T& second) const {
        return !std::invoke(compare_, first, second) && !std::invoke(compare_, second, first);
    }
    [[nodiscard]] auto lower_bound(const T& value) const {
        return std::lower_bound(values_.begin(), values_.end(), value, compare_);
    }
};

template <std::ranges::input_range Range>
coordinate_compressor(Range&&)
    -> coordinate_compressor<std::ranges::range_value_t<Range>>;

template <std::ranges::input_range Range, class Compare>
coordinate_compressor(Range&&, Compare)
    -> coordinate_compressor<std::ranges::range_value_t<Range>, std::decay_t<Compare>>;

} // namespace cp
