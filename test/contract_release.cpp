#include <cp/contract>

#include <tst.hpp>

int main() {
    return tst::run("contract release", [] {
        int condition_calls = 0;
        int message_calls = 0;
        CP_EXPECT(++condition_calls != 0, (++message_calls, "unused"));
        tst::check(condition_calls == 0,
                   "release contracts do not evaluate their condition");
        tst::check(message_calls == 0, "release contracts do not evaluate their message");
    });
}
