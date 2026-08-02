#include <cp/contract>

#include <tst.hpp>

#include <string_view>

namespace {

void test_contract() {
    int condition_calls = 0;
    int message_calls = 0;
    CP_EXPECT(++condition_calls == 1, (++message_calls, "unused"));
    tst::check(condition_calls == 1, "LOCAL evaluates a true condition once");
    tst::check(message_calls == 0, "a satisfied contract does not evaluate its message");
}

int contract_case(std::string_view name) {
    if (name == "failure") CP_EXPECT(false, "contract test failure");
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2) return contract_case(argv[1]);
    if (argc != 1) return 2;
    return tst::run("contract", test_contract);
}
