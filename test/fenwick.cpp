#include <cp/fenwick>

#include <tst.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

template <class Integer> constexpr std::size_t offset(Integer index) {
    return static_cast<std::size_t>(index);
}

template <class Random, class Integer>
Integer pick(Random& random, Integer bound) {
    return static_cast<Integer>(random() % static_cast<std::uint32_t>(bound));
}

struct rvalue_integer {
    int value;

    operator int() && { return value; }
    operator int() const& = delete;
};

template <class Range>
concept deduces_fenwick_tree = requires(Range&& values) {
    cp::fenwick_tree(std::forward<Range>(values));
};

static_assert(std::constructible_from<cp::fenwick_tree<cp::i64>, std::vector<int>&>);
static_assert(!std::constructible_from<cp::fenwick_tree<cp::i64>,
                                       std::vector<std::string>&>);
static_assert(!deduces_fenwick_tree<std::vector<int>&>);

long long naive_sum(const std::vector<long long>& values, cp::index_type first,
                    cp::index_type last) {
    long long result = 0;
    for (cp::index_type index = first; index < last; ++index) result += values[offset(index)];
    return result;
}

void test_fenwick_tree() {
    cp::fenwick_tree<long long> empty;
    tst::check(empty.empty() && empty.prefix_sum(0) == 0, "empty Fenwick tree");
    const cp::fenwick_tree<int> five_zeros{5};
    tst::check(five_zeros.size() == 5 && five_zeros.sum(0, 5) == 0,
               "count construction uses five zeroes");

    const std::array<int, 5> compatible_values{std::numeric_limits<int>::max(),
                                                std::numeric_limits<int>::max(), 4, 1, 5};
    const cp::fenwick_tree<cp::i64> compatible(compatible_values);
    tst::check(compatible.sum(0, 2) ==
                   2 * static_cast<cp::i64>(std::numeric_limits<int>::max()),
               "compatible values use the requested wide type");
    const std::list<int> noncontiguous{1, 2, 3, 4};
    const cp::fenwick_tree<cp::i64> from_list(noncontiguous);
    tst::check(from_list.sum(0, 4) == 10, "noncontiguous sized range");

    std::array<rvalue_integer, 3> movable_values{{{2}, {7}, {1}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::fenwick_tree<int> moved(moved_values);
    tst::check(moved.sum(0, 3) == 10, "rvalue-reference range conversion");

    constexpr cp::index_type size = 67;
    std::vector<long long> values(offset(size));
    for (cp::index_type index = 0; index < size; ++index) values[offset(index)] = index % 7 - 3;
    cp::fenwick_tree<long long> tree(values);
    std::mt19937 random(0xF311B17U);

    for (int step = 0; step < 8'000; ++step) {
        const cp::index_type position = pick(random, size);
        if (step % 3 == 0) {
            const long long delta = static_cast<long long>(random() % 2'001U) - 1'000;
            values[offset(position)] += delta;
            tree.add(position, delta);
        } else if (step % 3 == 1) {
            const long long value = static_cast<long long>(random() % 2'001U) - 1'000;
            values[offset(position)] = value;
            tree.set(position, value);
        } else {
            cp::index_type first = static_cast<cp::index_type>(random() % (size + 1));
            cp::index_type last = static_cast<cp::index_type>(random() % (size + 1));
            if (first > last) std::swap(first, last);
            tst::check(tree.prefix_sum(last) == naive_sum(values, 0, last),
                       "randomized prefix sum");
            tst::check(tree.sum(first, last) == naive_sum(values, first, last),
                       "randomized range sum");
            tst::check(tree.get(position) == values[offset(position)],
                       "randomized point value");
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "invalid-index")
        static_cast<void>(cp::fenwick_tree<int>(1).get(-1));
    if (argc != 1) return 2;
    return tst::run("fenwick", test_fenwick_tree);
}
