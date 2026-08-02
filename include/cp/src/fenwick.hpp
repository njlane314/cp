#pragma once

#include <cp/src/contract.hpp>
#include <cp/src/types.hpp>

#include <concepts>
#include <cstddef>
#include <limits>
#include <ranges>
#include <utility>
#include <vector>

namespace cp {

// Zero-based additive Fenwick tree; all ranges are half-open. T must support
// T{}, +=, and subtraction. Indices and boundaries are checked under LOCAL.
template <class T> class fenwick_tree {
  public:
    using value_type = T;

    fenwick_tree() : data_(1, T{}) {}
    explicit fenwick_tree(index_type count)
        : size_(checked_size(count)), data_(offset(size_) + 1, T{}) {}

    template <std::ranges::input_range Range>
        requires std::ranges::sized_range<Range> &&
                 std::convertible_to<std::ranges::range_reference_t<Range>, T>
    explicit fenwick_tree(Range&& values)
        : fenwick_tree(
              checked_size(static_cast<std::size_t>(std::ranges::size(values)))) {
        std::size_t position = 1;
        for (auto&& value : values)
            data_[position++] = static_cast<T>(std::forward<decltype(value)>(value));
        for (std::size_t i = 1; i <= offset(size_); ++i) {
            const std::size_t parent = i + low_bit(i);
            if (parent <= offset(size_)) data_[parent] += data_[i];
        }
    }

    [[nodiscard]] index_type size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void add(index_type position, const T& delta) {
        expect_element(position);
        for (std::size_t i = offset(position) + 1; i < data_.size(); i += low_bit(i))
            data_[i] += delta;
    }

    [[nodiscard]] T prefix_sum(index_type last) const {
        expect_boundary(last);
        T result{};
        for (std::size_t i = offset(last); i > 0; i -= low_bit(i)) result += data_[i];
        return result;
    }
    [[nodiscard]] T sum(index_type first, index_type last) const {
        expect_range(first, last);
        return prefix_sum(last) - prefix_sum(first);
    }
    [[nodiscard]] T get(index_type position) const {
        expect_element(position);
        return sum(position, position + 1);
    }
    void set(index_type position, const T& value) { add(position, value - get(position)); }

  private:
    index_type size_ = 0;
    std::vector<T> data_;

    static index_type checked_size(index_type count) {
        CP_EXPECT(count >= 0, "fenwick_tree: negative size");
        return count < 0 ? 0 : count;
    }
    static index_type checked_size(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<index_type>::max()))
            detail::contract_fail("count <= max(index_type)", "fenwick_tree: input is too large");
        return static_cast<index_type>(count);
    }
    static std::size_t offset(index_type index) noexcept {
        return static_cast<std::size_t>(index);
    }
    static std::size_t low_bit(std::size_t value) noexcept { return value & (~value + 1); }
    void expect_element(index_type position) const {
        CP_EXPECT(0 <= position && position < size_, "fenwick_tree: invalid position");
    }
    void expect_boundary(index_type boundary) const {
        CP_EXPECT(0 <= boundary && boundary <= size_,
                  "fenwick_tree::prefix_sum: invalid boundary");
    }
    void expect_range(index_type first, index_type last) const {
        CP_EXPECT(0 <= first && first <= last && last <= size_,
                  "fenwick_tree::sum: invalid range");
    }
};

} // namespace cp
