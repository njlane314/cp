#include <cp/segment>

#include <tst.hpp>

#include <algorithm>
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
concept deduces_segment_tree = requires(Range&& values) {
    cp::segment_tree(std::forward<Range>(values));
};

static_assert(std::constructible_from<cp::segment_tree<cp::i64>, std::vector<int>&>);
static_assert(!std::constructible_from<cp::segment_tree<cp::i64>,
                                       std::vector<std::string>&>);
static_assert(!deduces_segment_tree<std::vector<int>&>);

struct aggregate_node {
    cp::i64 sum = 0;
    int length = 0;

    friend bool operator==(const aggregate_node&, const aggregate_node&) = default;
};

struct concatenate {
    std::string operator()(const std::string& left, const std::string& right) const {
        return left + right;
    }
};

std::string naive_fold(const std::vector<std::string>& values, cp::index_type first,
                       cp::index_type last) {
    std::string result;
    for (cp::index_type index = first; index < last; ++index) result += values[offset(index)];
    return result;
}

void test_segment_tree() {
    cp::segment_tree<std::string, concatenate> empty(0, "", {});
    tst::check(empty.empty() && empty.fold(0, 0).empty() && empty.fold_all().empty(),
               "empty segment tree");
    const cp::segment_tree<int> five_zeros{5};
    tst::check(five_zeros.size() == 5 && five_zeros.fold_all() == 0,
               "count construction uses five zeroes");

    const int raw_values[] = {8, 6, 7, 5, 3, 0, 9};
    const cp::segment_tree<cp::i64> compatible(raw_values);
    tst::check(compatible.fold(1, 5) == 21, "compatible range conversion");
    const std::list<int> noncontiguous{1, 2, 3, 4};
    const cp::segment_tree<cp::i64> from_list(noncontiguous);
    tst::check(from_list.fold(0, 4) == 10, "noncontiguous sized range");

    auto minimum = cp::make_segment_tree(
        raw_values, std::numeric_limits<int>::max(),
        [](int first, int second) { return std::min(first, second); });
    static_assert(std::same_as<decltype(minimum)::value_type, int>);
    tst::check(minimum.fold(2, 6) == 0, "custom minimum operation");
    minimum.set(5, 10);
    tst::check(minimum.fold(2, 6) == 3, "custom operation after replacement");

    const std::vector<aggregate_node> nodes{{.sum = 4, .length = 1},
                                             {.sum = -2, .length = 1},
                                             {.sum = 9, .length = 1}};
    const auto combine_nodes = [](const aggregate_node& left, const aggregate_node& right) {
        return aggregate_node{.sum = left.sum + right.sum,
                              .length = left.length + right.length};
    };
    const auto node_tree = cp::make_segment_tree(nodes, aggregate_node{}, combine_nodes);
    tst::check(node_tree.fold(0, 3) == aggregate_node{.sum = 11, .length = 3},
               "custom aggregate nodes");
    tst::check(node_tree.fold(1, 1) == aggregate_node{}, "custom-node identity");

    std::array<rvalue_integer, 3> movable_values{{{6}, {2}, {5}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::segment_tree<int> moved(moved_values);
    tst::check(moved.fold(0, 3) == 13, "rvalue-reference range conversion");

    constexpr cp::index_type size = 37;
    std::vector<std::string> values(offset(size));
    for (cp::index_type index = 0; index < size; ++index)
        values[offset(index)] = std::string(1, static_cast<char>('a' + index % 5));
    cp::segment_tree<std::string, concatenate> tree(values, "", {});
    std::mt19937 random(0x5E67A11U);

    for (int step = 0; step < 6'000; ++step) {
        if (step % 3 == 0) {
            const cp::index_type position = pick(random, size);
            values[offset(position)] =
                std::string(1, static_cast<char>('A' + random() % 26U));
            tree.set(position, values[offset(position)]);
            tst::check(tree.get(position) == values[offset(position)],
                       "randomized point replacement");
        } else {
            cp::index_type first = static_cast<cp::index_type>(random() % (size + 1));
            cp::index_type last = static_cast<cp::index_type>(random() % (size + 1));
            if (first > last) std::swap(first, last);
            tst::check(tree.fold(first, last) == naive_fold(values, first, last),
                       "randomized order-preserving fold");
            tst::check(tree.fold_all() == naive_fold(values, 0, size),
                       "randomized full fold");
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "invalid-range")
        static_cast<void>(cp::segment_tree<int>(1).fold(1, 0));
    if (argc != 1) return 2;
    return tst::run("segment", test_segment_tree);
}
