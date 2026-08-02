#include <cp/compressor>
#include <tst.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

inline constexpr std::uint32_t random_seed = 0xC00D1A7EU;

struct rvalue_integer {
    int value;

    operator int() && { return value; }
    operator int() const& = delete;
};

struct oversized_range {
    [[nodiscard]] const int* begin() const noexcept { return &storage; }
    [[nodiscard]] const int* end() const noexcept { return &storage; }
    [[nodiscard]] std::size_t size() const noexcept {
        return static_cast<std::size_t>(std::numeric_limits<cp::index_type>::max()) + 1;
    }

    int storage = 0;
};

void check_random(bool condition, std::string_view message, int step) {
    if (condition) return;
    std::ostringstream detail;
    detail << message << " (seed=0xC00D1A7E, step=" << step << ')';
    tst::check(false, detail.str());
}

void test_compressor() {
    const cp::coordinate_compressor<int> empty;
    tst::check(empty.empty(), "default compressor is empty");
    tst::check(empty.size() == 0, "default compressor has size zero");

    const std::vector<int> values{40, 10, 40, -5, 10, 90};
    const cp::coordinate_compressor coordinates(values);
    tst::check(coordinates.size() == 4, "construction removes duplicate values");
    tst::check(coordinates.rank(-5) == 0, "smallest value has rank zero");
    tst::check(coordinates.rank(10) == 1, "rank follows sorted order");
    tst::check(coordinates.rank(40) == 2, "duplicate input retains one rank");
    tst::check(coordinates.rank(90) == 3, "largest value has the final rank");
    tst::check(coordinates.value(2) == 40, "value reverses rank");

    const cp::coordinate_compressor<cp::i64> widened(values);
    tst::check(widened.rank(90) == 3, "constructor converts compatible values");
    tst::check(widened.value(0) == -5, "converted compressor preserves values");

    std::istringstream input("7 2 7 -4 2");
    auto input_values = std::ranges::istream_view<int>(input);
    const cp::coordinate_compressor streamed(input_values);
    tst::check(streamed.size() == 3, "constructor accepts a one-pass range");
    tst::check(streamed.value(0) == -4, "one-pass range is sorted");
    tst::check(streamed.rank(7) == 2, "one-pass range ranks remain searchable");

    std::array<rvalue_integer, 4> movable_values{{{8}, {3}, {8}, {-1}}};
    auto moved_values = std::ranges::subrange(
        std::make_move_iterator(movable_values.begin()),
        std::make_move_iterator(movable_values.end()));
    const cp::coordinate_compressor<int> moved(moved_values);
    tst::check(moved.size() == 3, "constructor accepts rvalue-only conversion");
    tst::check(moved.value(1) == 3, "rvalue range values are forwarded");

    const std::vector<bool> flags{true, false, true};
    const cp::coordinate_compressor flag_values(flags);
    tst::check(flag_values.size() == 2, "bool values are compressed");
    tst::check(!flag_values.value(0), "false sorts before true");
    tst::check(flag_values.value(1), "bool value returns without a dangling reference");

    const auto descending_compare = [](int first, int second) { return first > second; };
    const cp::coordinate_compressor descending(values, descending_compare);
    tst::check(descending.value(0) == 90, "custom comparator controls value order");
    tst::check(descending.rank(-5) == 3, "custom comparator controls ranks");

    std::mt19937 random(random_seed);
    for (int step = 0; step < 1'000; ++step) {
        std::vector<int> sample(random() % 80U);
        for (int& value : sample) value = static_cast<int>(random() % 101U) - 50;

        std::vector<int> expected = sample;
        std::sort(expected.begin(), expected.end());
        expected.erase(std::unique(expected.begin(), expected.end()), expected.end());

        const cp::coordinate_compressor compressed(sample);
        check_random(compressed.size() == static_cast<cp::index_type>(expected.size()),
                     "randomized distinct count differs from sorted unique", step);
        for (std::size_t index = 0; index < expected.size(); ++index) {
            check_random(compressed.value(static_cast<cp::index_type>(index)) == expected[index],
                         "randomized value differs from sorted unique", step);
            check_random(compressed.rank(expected[index]) ==
                             static_cast<cp::index_type>(index),
                         "randomized rank differs from sorted unique", step);
        }
    }
}

int contract_case(std::string_view name) {
    if (name == "missing-rank") {
        const cp::coordinate_compressor coordinates(std::array{1, 2, 3});
        static_cast<void>(coordinates.rank(4));
    } else if (name == "invalid-value") {
        const cp::coordinate_compressor coordinates(std::array{1, 2, 3});
        static_cast<void>(coordinates.value(3));
    } else if (name == "oversized-input") {
        const cp::coordinate_compressor coordinates(oversized_range{});
        static_cast<void>(coordinates);
    }
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2) return contract_case(argv[1]);
    if (argc != 1) return 2;
    return tst::run("compressor", test_compressor);
}
