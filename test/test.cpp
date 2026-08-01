#define LOCAL

#include <cp/contract.hpp>
#include <cp/coordinate_compressor.hpp>
#include <cp/disjoint_set.hpp>
#include <cp/fenwick_tree.hpp>
#include <cp/kmp_matcher.hpp>
#include <cp/modint.hpp>
#include <cp/recursive.hpp>
#include <cp/segment_tree.hpp>
#include <cp/types.hpp>
#include <cp/utility.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <random>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr bool utility_checks() {
    int low = 8;
    int high = 8;
    return cp::chmin(low, 3) && !cp::chmin(low, 5) && cp::chmax(high, 13) &&
           !cp::chmax(high, 9) && low == 3 && high == 13;
}

using mint = cp::modint<1'000'000'007U>;

template <class Range>
concept deduces_fenwick_tree = requires(Range&& values) {
    cp::fenwick_tree(std::forward<Range>(values));
};

template <class Range>
concept deduces_segment_tree = requires(Range&& values) {
    cp::segment_tree(std::forward<Range>(values));
};

template <class T>
concept supports_division = requires(T first, T second) { first / second; };

static_assert(utility_checks());
static_assert(cp::npos == -1 && sizeof(cp::i64) == 8 && sizeof(cp::u64) == 8);
static_assert(mint::modulus() == 1'000'000'007U);
static_assert(mint{-1}.value() == 1'000'000'006U);
static_assert((mint{10} + mint{20}).value() == 30);
static_assert(mint{2}.pow(10).value() == 1'024);
static_assert(mint{2}.try_inverse().has_value());
static_assert((mint{2} * *mint{2}.try_inverse()).value() == 1);
static_assert(!supports_division<mint>);
static_assert(std::constructible_from<cp::fenwick_tree<cp::i64>, std::vector<int>&>);
static_assert(std::constructible_from<cp::segment_tree<cp::i64>, std::vector<int>&>);
static_assert(!std::constructible_from<cp::fenwick_tree<cp::i64>, std::vector<std::string>&>);
static_assert(!std::constructible_from<cp::segment_tree<cp::i64>, std::vector<std::string>&>);
static_assert(!deduces_fenwick_tree<std::vector<int>&>);
static_assert(!deduces_segment_tree<std::vector<int>&>);
#if defined(__SIZEOF_INT128__)
constexpr cp::u128 huge = (static_cast<cp::u128>(1) << 100) + 7;
constexpr auto huge_remainder = static_cast<std::uint32_t>(huge % mint::modulus());
static_assert(mint{huge}.value() == huge_remainder);
static_assert(mint{-static_cast<cp::i128>(huge)}.value() == mint::modulus() - huge_remainder);
#endif

std::size_t offset(cp::index_type index) { return static_cast<std::size_t>(index); }

cp::index_type pick(std::mt19937& random, cp::index_type bound) {
    return static_cast<cp::index_type>(random() % static_cast<std::uint32_t>(bound));
}

struct rvalue_integer {
    int value;

    operator int() && { return value; }
    operator int() const& = delete;
};

struct aggregate_node {
    cp::i64 sum = 0;
    int length = 0;

    friend bool operator==(const aggregate_node&, const aggregate_node&) = default;
};

struct oversized_range {
    [[nodiscard]] const int* begin() const noexcept { return &storage; }
    [[nodiscard]] const int* end() const noexcept { return &storage; }
    [[nodiscard]] std::size_t size() const noexcept {
        return static_cast<std::size_t>(std::numeric_limits<cp::index_type>::max()) + 1;
    }

    int storage = 0;
};

void test_contract() {
    int message_calls = 0;
    CP_EXPECT(2 + 2 == 4, (++message_calls, "unused"));
    assert(message_calls == 0);
}

void test_disjoint_set() {
    cp::disjoint_set empty;
    assert(empty.empty() && empty.size() == 0 && empty.component_count() == 0);

    cp::disjoint_set sample(5);
    sample.merge(0, 2);
    sample.merge(3, 4);
    const cp::disjoint_set& sample_view = sample;
    assert(sample_view.same(0, 2));
    assert(sample_view.component_size(0) == 2);
    assert(sample_view.groups() ==
           std::vector<std::vector<cp::index_type>>({{0, 2}, {1}, {3, 4}}));

    constexpr cp::index_type size = 41;
    cp::disjoint_set sets(size);
    std::vector<cp::index_type> label(offset(size));
    std::iota(label.begin(), label.end(), 0);
    std::mt19937 random(0xD515EA5EU);

    for (int step = 0; step < 5'000; ++step) {
        const cp::index_type first = pick(random, size);
        const cp::index_type second = pick(random, size);
        const bool were_same = label[offset(first)] == label[offset(second)];
        if (step % 3 == 0) {
            assert(sets.same(first, second) == were_same);
        } else {
            assert(sets.merge(first, second) == !were_same);
            if (!were_same) {
                const auto replaced = label[offset(second)];
                const auto replacement = label[offset(first)];
                std::replace(label.begin(), label.end(), replaced, replacement);
            }
        }

        const cp::index_type element = pick(random, size);
        const auto expected = std::count(label.begin(), label.end(), label[offset(element)]);
        assert(sets.component_size(element) == static_cast<cp::index_type>(expected));
        std::vector<cp::index_type> distinct = label;
        std::sort(distinct.begin(), distinct.end());
        distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
        assert(sets.component_count() == static_cast<cp::index_type>(distinct.size()));
    }
}

long long naive_sum(const std::vector<long long>& values, cp::index_type first,
                    cp::index_type last) {
    long long result = 0;
    for (cp::index_type index = first; index < last; ++index) result += values[offset(index)];
    return result;
}

void test_fenwick_tree() {
    cp::fenwick_tree<long long> empty;
    assert(empty.empty() && empty.prefix_sum(0) == 0);
    const cp::fenwick_tree<int> five_zeros{5};
    assert(five_zeros.size() == 5 && five_zeros.sum(0, 5) == 0);

    const std::array<int, 5> compatible_values{std::numeric_limits<int>::max(),
                                                std::numeric_limits<int>::max(), 4, 1, 5};
    const cp::fenwick_tree<cp::i64> compatible(compatible_values);
    assert(compatible.sum(0, 2) ==
           2 * static_cast<cp::i64>(std::numeric_limits<int>::max()));
    const std::list<int> noncontiguous{1, 2, 3, 4};
    const cp::fenwick_tree<cp::i64> from_list(noncontiguous);
    assert(from_list.sum(0, 4) == 10);

    std::array<rvalue_integer, 3> movable_values{{{2}, {7}, {1}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::fenwick_tree<int> moved(moved_values);
    assert(moved.sum(0, 3) == 10);

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
            assert(tree.prefix_sum(last) == naive_sum(values, 0, last));
            assert(tree.sum(first, last) == naive_sum(values, first, last));
            assert(tree.get(position) == values[offset(position)]);
        }
    }
}

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
    assert(empty.empty() && empty.fold(0, 0).empty() && empty.fold_all().empty());
    const cp::segment_tree<int> five_zeros{5};
    assert(five_zeros.size() == 5 && five_zeros.fold_all() == 0);

    const int raw_values[] = {8, 6, 7, 5, 3, 0, 9};
    const cp::segment_tree<cp::i64> compatible(raw_values);
    assert(compatible.fold(1, 5) == 21);
    const std::list<int> noncontiguous{1, 2, 3, 4};
    const cp::segment_tree<cp::i64> from_list(noncontiguous);
    assert(from_list.fold(0, 4) == 10);

    auto minimum = cp::make_segment_tree(
        raw_values, std::numeric_limits<int>::max(),
        [](int first, int second) { return std::min(first, second); });
    static_assert(std::same_as<decltype(minimum)::value_type, int>);
    assert(minimum.fold(2, 6) == 0);
    minimum.set(5, 10);
    assert(minimum.fold(2, 6) == 3);

    const std::vector<aggregate_node> nodes{{.sum = 4, .length = 1},
                                             {.sum = -2, .length = 1},
                                             {.sum = 9, .length = 1}};
    const auto combine_nodes = [](const aggregate_node& left, const aggregate_node& right) {
        return aggregate_node{.sum = left.sum + right.sum,
                              .length = left.length + right.length};
    };
    const auto node_tree = cp::make_segment_tree(nodes, aggregate_node{}, combine_nodes);
    assert(node_tree.fold(0, 3) == aggregate_node({.sum = 11, .length = 3}));
    assert(node_tree.fold(1, 1) == aggregate_node{});

    std::array<rvalue_integer, 3> movable_values{{{6}, {2}, {5}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::segment_tree<int> moved(moved_values);
    assert(moved.fold(0, 3) == 13);

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
            assert(tree.get(position) == values[offset(position)]);
        } else {
            cp::index_type first = static_cast<cp::index_type>(random() % (size + 1));
            cp::index_type last = static_cast<cp::index_type>(random() % (size + 1));
            if (first > last) std::swap(first, last);
            assert(tree.fold(first, last) == naive_fold(values, first, last));
            assert(tree.fold_all() == naive_fold(values, 0, size));
        }
    }
}

void test_modint() {
    constexpr std::uint64_t modulus = mint::modulus();
    std::mt19937_64 random(0xBADC0FFEEULL);
    for (int step = 0; step < 10'000; ++step) {
        const std::uint64_t raw_first = random();
        const std::uint64_t raw_second = random();
        const std::uint64_t first = raw_first % modulus;
        const std::uint64_t second = raw_second % modulus;
        const mint a = raw_first;
        const mint b = raw_second;
        assert((a + b).value() == (first + second) % modulus);
        assert((a - b).value() == (first + modulus - second) % modulus);
        assert((a * b).value() == first * second % modulus);
        if (second != 0) {
            const auto inverse = b.try_inverse();
            assert(inverse.has_value());
            assert((a * *inverse) * b == a);
        }
    }
    assert(cp::modint<12>{5}.try_inverse()->value() == 5);
    assert(!cp::modint<12>{0}.try_inverse().has_value());
    assert(!cp::modint<12>{6}.try_inverse().has_value());
    using wide = cp::modint<4'294'967'291U>;
    const wide minus_one = 4'294'967'290ULL;
    assert((minus_one * minus_one).value() == 1);
}

void test_recursive() {
    const auto factorial = cp::recursive([](const auto& self, int value) -> int {
        return value <= 1 ? 1 : value * self(value - 1);
    });
    assert(factorial(0) == 1);
    assert(factorial(10) == 3'628'800);

    const std::vector<std::vector<int>> graph{{1, 2}, {0, 3, 4}, {0}, {1}, {1}};
    std::vector<int> order;
    auto dfs = cp::recursive([&](auto& self, int vertex, int parent) -> void {
        order.push_back(vertex);
        for (int next : graph[offset(vertex)])
            if (next != parent) self(next, vertex);
    });
    dfs(0, -1);
    assert(order == std::vector<int>({0, 1, 3, 4, 2}));

    auto move_only = cp::recursive(
        [factor = std::make_unique<int>(2)](auto& self, int exponent) mutable -> int {
            return exponent == 0 ? 1 : *factor * self(exponent - 1);
        });
    assert(move_only(8) == 256);

    std::mt19937 random(0xA11CE123U);
    for (int trial = 0; trial < 200; ++trial) {
        const int size = 1 + static_cast<int>(random() % 80U);
        std::vector<int> parent(static_cast<std::size_t>(size), -1);
        std::vector<std::vector<int>> tree(static_cast<std::size_t>(size));
        for (int vertex = 1; vertex < size; ++vertex) {
            parent[offset(vertex)] = static_cast<int>(random() % static_cast<unsigned>(vertex));
            tree[offset(parent[offset(vertex)])].push_back(vertex);
        }

        std::vector<int> expected(static_cast<std::size_t>(size), 1);
        for (int vertex = size - 1; vertex > 0; --vertex)
            expected[offset(parent[offset(vertex)])] += expected[offset(vertex)];

        std::vector<int> actual(static_cast<std::size_t>(size));
        auto subtree_size = cp::recursive([&](auto& self, int vertex) -> int {
            int result = 1;
            for (int child : tree[offset(vertex)]) result += self(child);
            return actual[offset(vertex)] = result;
        });
        assert(subtree_size(0) == size);
        assert(actual == expected);
    }
}

void test_coordinate_compressor() {
    const cp::coordinate_compressor<int> empty;
    assert(empty.empty() && empty.size() == 0);

    const std::vector<int> values{40, 10, 40, -5, 10, 90};
    const cp::coordinate_compressor coordinates(values);
    assert(coordinates.size() == 4);
    assert(coordinates.rank(-5) == 0);
    assert(coordinates.rank(10) == 1);
    assert(coordinates.rank(40) == 2);
    assert(coordinates.rank(90) == 3);
    assert(coordinates.value(2) == 40);

    const cp::coordinate_compressor<cp::i64> widened(values);
    assert(widened.rank(90) == 3 && widened.value(0) == -5);

    std::istringstream input("7 2 7 -4 2");
    auto input_values = std::ranges::istream_view<int>(input);
    const cp::coordinate_compressor streamed(input_values);
    assert(streamed.size() == 3 && streamed.value(0) == -4 && streamed.rank(7) == 2);

    std::array<rvalue_integer, 4> movable_values{{{8}, {3}, {8}, {-1}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::coordinate_compressor<int> moved(moved_values);
    assert(moved.size() == 3 && moved.value(1) == 3);

    const std::vector<bool> flags{true, false, true};
    const cp::coordinate_compressor flag_values(flags);
    assert(flag_values.size() == 2 && !flag_values.value(0) && flag_values.value(1));

    const auto descending_compare = [](int first, int second) { return first > second; };
    const cp::coordinate_compressor descending(values, descending_compare);
    assert(descending.value(0) == 90 && descending.rank(-5) == 3);

    std::mt19937 random(0xC00D1A7EU);
    for (int step = 0; step < 1'000; ++step) {
        std::vector<int> sample(random() % 80U);
        for (int& value : sample) value = static_cast<int>(random() % 101U) - 50;

        std::vector<int> expected = sample;
        std::sort(expected.begin(), expected.end());
        expected.erase(std::unique(expected.begin(), expected.end()), expected.end());

        const cp::coordinate_compressor compressed(sample);
        assert(compressed.size() == static_cast<cp::index_type>(expected.size()));
        for (std::size_t index = 0; index < expected.size(); ++index) {
            assert(compressed.value(static_cast<cp::index_type>(index)) == expected[index]);
            assert(compressed.rank(expected[index]) == static_cast<cp::index_type>(index));
        }
    }
}

std::vector<cp::index_type> brute_matches(std::string_view text, std::string_view pattern) {
    std::vector<cp::index_type> matches;
    for (std::size_t index = 0; index + pattern.size() <= text.size(); ++index)
        if (text.substr(index, pattern.size()) == pattern)
            matches.push_back(static_cast<cp::index_type>(index));
    return matches;
}

std::string random_string(std::mt19937& random, std::size_t length) {
    std::string result(length, 'a');
    for (char& character : result) character = static_cast<char>('a' + random() % 3U);
    return result;
}

void test_kmp() {
    const cp::kmp_matcher matcher("ababcabab");
    assert(matcher.size() == 9 && !matcher.empty() && matcher.pattern() == "ababcabab");
    assert(matcher.prefix_table() ==
           std::vector<cp::index_type>({0, 0, 1, 2, 0, 1, 2, 3, 4}));
    assert(cp::kmp_matcher("aaa").find_all("aaaaa") ==
           std::vector<cp::index_type>({0, 1, 2}));
    const cp::kmp_matcher empty("");
    assert(empty.empty() && empty.size() == 0);
    assert(empty.find_all("abc") == std::vector<cp::index_type>({0, 1, 2, 3}));

    std::mt19937 random(0x4B4D50U);
    for (int step = 0; step < 10'000; ++step) {
        const std::string text = random_string(random, random() % 31U);
        const std::string pattern = random_string(random, random() % 11U);
        assert(cp::kmp_matcher(pattern).find_all(text) == brute_matches(text, pattern));
    }
}

int contract_case(std::string_view name) {
    if (name == "disjoint-size") {
        const cp::disjoint_set invalid(-1);
        static_cast<void>(invalid);
    } else if (name == "fenwick-index") {
        static_cast<void>(cp::fenwick_tree<int>(1).get(-1));
    } else if (name == "segment-range") {
        static_cast<void>(cp::segment_tree<int>(1).fold(1, 0));
    } else if (name == "compressor-rank") {
        const cp::coordinate_compressor coordinates(std::array{1, 2, 3});
        static_cast<void>(coordinates.rank(4));
    } else if (name == "compressor-value") {
        const cp::coordinate_compressor coordinates(std::array{1, 2, 3});
        static_cast<void>(coordinates.value(3));
    } else if (name == "compressor-size") {
        const cp::coordinate_compressor coordinates(oversized_range{});
        static_cast<void>(coordinates);
    }
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2) return contract_case(argv[1]);
    test_contract();
    test_disjoint_set();
    test_fenwick_tree();
    test_segment_tree();
    test_modint();
    test_kmp();
    test_recursive();
    test_coordinate_compressor();
}
