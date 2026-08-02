#include <cp/disjoint>

#include <tst.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <string_view>
#include <vector>

namespace {

template <class Integer> constexpr std::size_t offset(Integer index) {
    return static_cast<std::size_t>(index);
}

template <class Random, class Integer>
Integer pick(Random& random, Integer bound) {
    return static_cast<Integer>(random() % static_cast<std::uint32_t>(bound));
}

void test_disjoint_set() {
    cp::disjoint_set empty;
    tst::check(empty.empty() && empty.size() == 0 && empty.component_count() == 0,
               "empty disjoint set");

    cp::disjoint_set sample(5);
    sample.merge(0, 2);
    sample.merge(3, 4);
    const cp::disjoint_set& sample_view = sample;
    tst::check(sample_view.same(0, 2), "const connectivity query");
    tst::check(sample_view.component_size(0) == 2, "const component-size query");
    tst::check(sample_view.groups() ==
                   std::vector<std::vector<cp::index_type>>({{0, 2}, {1}, {3, 4}}),
               "deterministic groups");

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
            tst::check(sets.same(first, second) == were_same,
                       "randomized connectivity query");
        } else {
            tst::check(sets.merge(first, second) == !were_same,
                       "randomized merge result");
            if (!were_same) {
                const auto replaced = label[offset(second)];
                const auto replacement = label[offset(first)];
                std::replace(label.begin(), label.end(), replaced, replacement);
            }
        }

        const cp::index_type element = pick(random, size);
        const auto expected = std::count(label.begin(), label.end(), label[offset(element)]);
        tst::check(sets.component_size(element) == static_cast<cp::index_type>(expected),
                   "randomized component size");
        std::vector<cp::index_type> distinct = label;
        std::sort(distinct.begin(), distinct.end());
        distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
        tst::check(sets.component_count() == static_cast<cp::index_type>(distinct.size()),
                   "randomized component count");
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "negative-size") {
        const cp::disjoint_set invalid(-1);
        static_cast<void>(invalid);
    }
    if (argc != 1) return 2;
    return tst::run("disjoint", test_disjoint_set);
}
