#pragma once

#include <cp/src/contract.hpp>
#include <cp/src/types.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace cp {

// Point replacements and order-preserving folds over half-open ranges. combine must
// be associative and identity must be a two-sided identity. LOCAL checks ranges.
template <class T, class BinaryOperation = std::plus<T>> class segment_tree {
  public:
    using value_type = T;
    using operation_type = BinaryOperation;

    explicit segment_tree(index_type count, T identity = T{}, BinaryOperation combine = {})
        : size_(checked_size(count)), base_(tree_base(size_)), identity_(std::move(identity)),
          combine_(std::move(combine)), data_(tree_storage(base_), identity_) {}

    template <std::ranges::input_range Range>
        requires std::ranges::sized_range<Range> &&
                 std::convertible_to<std::ranges::range_reference_t<Range>, T>
    explicit segment_tree(Range&& values, T identity = T{}, BinaryOperation combine = {})
        : segment_tree(
              checked_size(static_cast<std::size_t>(std::ranges::size(values))),
              std::move(identity), std::move(combine)) {
        std::size_t position = base_;
        for (auto&& value : values)
            data_[position++] = static_cast<T>(std::forward<decltype(value)>(value));
        for (std::size_t node = base_ - 1; node > 0; --node) pull(node);
    }

    [[nodiscard]] index_type size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] const T& get(index_type position) const {
        expect_element(position);
        return data_[base_ + offset(position)];
    }

    void set(index_type position, T value) {
        expect_element(position);
        std::size_t node = base_ + offset(position);
        data_[node] = std::move(value);
        while (node > 1) {
            node /= 2;
            pull(node);
        }
    }

    [[nodiscard]] T fold(index_type first, index_type last) const {
        expect_range(first, last);
        std::size_t left = base_ + offset(first);
        std::size_t right = base_ + offset(last);
        T left_result = identity_;
        T right_result = identity_;
        while (left < right) {
            if ((left & 1U) != 0)
                left_result = std::invoke(combine_, left_result, data_[left++]);
            if ((right & 1U) != 0)
                right_result = std::invoke(combine_, data_[--right], right_result);
            left /= 2;
            right /= 2;
        }
        return std::invoke(combine_, left_result, right_result);
    }
    [[nodiscard]] T fold_all() const { return data_[1]; }

  private:
    index_type size_ = 0;
    std::size_t base_ = 1;
    T identity_;
    [[no_unique_address]] BinaryOperation combine_;
    std::vector<T> data_;

    static index_type checked_size(index_type count) {
        CP_EXPECT(count >= 0, "segment_tree: negative size");
        return count < 0 ? 0 : count;
    }
    static index_type checked_size(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<index_type>::max()))
            detail::contract_fail("count <= max(index_type)", "segment_tree: input is too large");
        return static_cast<index_type>(count);
    }
    static std::size_t offset(index_type index) noexcept {
        return static_cast<std::size_t>(index);
    }
    static std::size_t tree_base(index_type count) {
        std::size_t base = 1;
        while (base < offset(count)) base *= 2;
        return base;
    }
    static std::size_t tree_storage(std::size_t base) {
        if (base > std::numeric_limits<std::size_t>::max() / 2)
            detail::contract_fail("base <= max(size_t) / 2", "segment_tree: input is too large");
        return 2 * base;
    }
    void pull(std::size_t node) {
        data_[node] = std::invoke(combine_, data_[2 * node], data_[2 * node + 1]);
    }
    void expect_element(index_type position) const {
        CP_EXPECT(0 <= position && position < size_, "segment_tree: invalid position");
    }
    void expect_range(index_type first, index_type last) const {
        CP_EXPECT(0 <= first && first <= last && last <= size_,
                  "segment_tree::fold: invalid range");
    }
};

template <std::ranges::input_range Range, class T, class BinaryOperation>
    requires std::ranges::sized_range<Range> &&
             std::convertible_to<std::ranges::range_reference_t<Range>, T> &&
             requires(const std::decay_t<BinaryOperation>& operation, const T& first,
                      const T& second) {
                 { std::invoke(operation, first, second) } -> std::convertible_to<T>;
             }
[[nodiscard]] auto make_segment_tree(Range&& values, T identity, BinaryOperation&& combine) {
    using operation_type = std::decay_t<BinaryOperation>;
    return segment_tree<T, operation_type>(std::forward<Range>(values), std::move(identity),
                                           std::forward<BinaryOperation>(combine));
}

} // namespace cp
