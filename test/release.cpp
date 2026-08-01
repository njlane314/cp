#include <cp/contract.hpp>
#include <cp/modint.hpp>

#include <cassert>

int main() {
    int evaluations = 0;
    CP_EXPECT(++evaluations != 0, (++evaluations, "unused"));
    assert(evaluations == 0);
    assert(cp::modint<12>{5}.try_inverse()->value() == 5);
    assert(!cp::modint<12>{6}.try_inverse().has_value());
}
