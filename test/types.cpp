#include <cp/types>

#include <tst.hpp>

#include <concepts>
#include <cstdint>

static_assert(cp::npos == -1);
static_assert(std::same_as<cp::index_type, int>);
static_assert(std::signed_integral<cp::index_type>);
static_assert(std::same_as<cp::i64, std::int64_t>);
static_assert(std::same_as<cp::u64, std::uint64_t>);

#if defined(__SIZEOF_INT128__)
static_assert(std::same_as<cp::i128, __int128_t>);
static_assert(std::same_as<cp::u128, __uint128_t>);
#endif

int main() {
    return tst::run("types", [] {
        tst::check(cp::npos == -1, "npos is the negative index sentinel");
        tst::check(cp::index_type{-1} < cp::index_type{0}, "index_type is signed");
        tst::check(sizeof(cp::i64) == 8, "i64 is 64 bits");
        tst::check(sizeof(cp::u64) == 8, "u64 is 64 bits");
    });
}
