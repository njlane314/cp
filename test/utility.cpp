#include <cp/utility>

#include <tst.hpp>

namespace {

constexpr bool utility_checks() {
    int low = 8;
    int high = 8;
    return cp::chmin(low, 3) && !cp::chmin(low, 5) && !cp::chmin(low, 3) &&
           cp::chmax(high, 13) && !cp::chmax(high, 9) && !cp::chmax(high, 13) && low == 3 &&
           high == 13;
}

static_assert(utility_checks());

} // namespace

int main() {
    return tst::run("utility", [] {
        tst::check(utility_checks(),
                   "chmin and chmax update only for strictly better candidates");
    });
}
