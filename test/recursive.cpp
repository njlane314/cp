#include <cp/recursive>
#include <tst.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

inline constexpr std::uint32_t random_seed = 0xA11CE123U;

[[nodiscard]] std::size_t offset(int index) { return static_cast<std::size_t>(index); }

void check_random(bool condition, std::string_view message, int trial) {
    if (condition) return;
    std::ostringstream detail;
    detail << message << " (seed=0xA11CE123, trial=" << trial << ')';
    tst::check(false, detail.str());
}

void test_recursive() {
    const auto factorial = cp::recursive([](const auto& self, int value) -> int {
        return value <= 1 ? 1 : value * self(value - 1);
    });
    tst::check(factorial(0) == 1, "recursive callable handles its base case");
    tst::check(factorial(10) == 3'628'800, "recursive callable returns a value");

    const std::vector<std::vector<int>> graph{{1, 2}, {0, 3, 4}, {0}, {1}, {1}};
    std::vector<int> order;
    auto dfs = cp::recursive([&](auto& self, int vertex, int parent) -> void {
        order.push_back(vertex);
        for (int next : graph[offset(vertex)])
            if (next != parent) self(next, vertex);
    });
    dfs(0, -1);
    tst::check(order == std::vector<int>({0, 1, 3, 4, 2}),
               "recursive callable supports void depth-first traversal");

    auto move_only = cp::recursive(
        [factor = std::make_unique<int>(2)](auto& self, int exponent) mutable -> int {
            return exponent == 0 ? 1 : *factor * self(exponent - 1);
        });
    tst::check(move_only(8) == 256, "recursive callable stores a move-only mutable closure");

    std::mt19937 random(random_seed);
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
        check_random(subtree_size(0) == size, "root subtree size differs from tree size", trial);
        check_random(actual == expected, "recursive subtree sizes differ from iterative result",
                     trial);
    }
}

} // namespace

int main() { return tst::run("recursive", test_recursive); }
