#include <cp/modint>

#include <tst.hpp>

#include <cstdint>
#include <random>

namespace {

using mint = cp::modint<1'000'000'007U>;

template <class T>
concept supports_division = requires(T first, T second) { first / second; };

static_assert(mint::modulus() == 1'000'000'007U);
static_assert(mint{-1}.value() == 1'000'000'006U);
static_assert((mint{10} + mint{20}).value() == 30);
static_assert(mint{2}.pow(10).value() == 1'024);
static_assert(mint{2}.try_inverse().has_value());
static_assert((mint{2} * *mint{2}.try_inverse()).value() == 1);
static_assert(!supports_division<mint>);

#if defined(__SIZEOF_INT128__)
constexpr cp::u128 huge = (static_cast<cp::u128>(1) << 100) + 7;
constexpr auto huge_remainder = static_cast<std::uint32_t>(huge % mint::modulus());
static_assert(mint{huge}.value() == huge_remainder);
static_assert(mint{-static_cast<cp::i128>(huge)}.value() == mint::modulus() - huge_remainder);
#endif

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
        tst::check((a + b).value() == (first + second) % modulus,
                   "modular addition");
        tst::check((a - b).value() == (first + modulus - second) % modulus,
                   "modular subtraction");
        tst::check((a * b).value() == first * second % modulus,
                   "modular multiplication");
        if (second != 0) {
            const auto inverse = b.try_inverse();
            tst::check(inverse.has_value(), "nonzero prime-field value is invertible");
            tst::check((a * *inverse) * b == a, "multiplication by an inverse round trips");
        }
    }

    tst::check(cp::modint<12>{5}.try_inverse()->value() == 5,
               "coprime composite-modulus value is invertible");
    tst::check(!cp::modint<12>{0}.try_inverse().has_value(), "zero is not invertible");
    tst::check(!cp::modint<12>{6}.try_inverse().has_value(),
               "non-coprime value is not invertible");

    using wide = cp::modint<4'294'967'291U>;
    const wide minus_one = 4'294'967'290ULL;
    tst::check((minus_one * minus_one).value() == 1,
               "multiplication supports a near-32-bit modulus");
}

} // namespace

int main() { return tst::run("modint", test_modint); }
