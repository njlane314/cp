#include <cp/kmp>
#include <tst.hpp>

#include <cstddef>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

inline constexpr std::uint32_t random_seed = 0x4B4D50U;

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

void check_random(bool condition, int step, std::string_view text, std::string_view pattern) {
    if (condition) return;
    std::ostringstream detail;
    detail << "find_all differs from brute force (seed=0x4B4D50, step=" << step
           << ", text=\"" << text << "\", pattern=\"" << pattern << "\")";
    tst::check(false, detail.str());
}

void test_kmp() {
    const cp::kmp_matcher matcher("ababcabab");
    tst::check(matcher.size() == 9, "matcher reports its pattern size");
    tst::check(!matcher.empty(), "nonempty pattern produces a nonempty matcher");
    tst::check(matcher.pattern() == "ababcabab", "matcher retains its pattern");
    tst::check(matcher.prefix_table() ==
                   std::vector<cp::index_type>({0, 0, 1, 2, 0, 1, 2, 3, 4}),
               "prefix table matches the known example");
    tst::check(cp::kmp_matcher("aaa").find_all("aaaaa") ==
                   std::vector<cp::index_type>({0, 1, 2}),
               "find_all includes overlapping matches");

    const cp::kmp_matcher empty("");
    tst::check(empty.empty(), "empty pattern produces an empty matcher");
    tst::check(empty.size() == 0, "empty matcher has size zero");
    tst::check(empty.find_all("abc") == std::vector<cp::index_type>({0, 1, 2, 3}),
               "empty pattern matches every boundary");

    std::mt19937 random(random_seed);
    for (int step = 0; step < 10'000; ++step) {
        const std::string text = random_string(random, random() % 31U);
        const std::string pattern = random_string(random, random() % 11U);
        check_random(cp::kmp_matcher(pattern).find_all(text) == brute_matches(text, pattern),
                     step, text, pattern);
    }
}

} // namespace

int main() { return tst::run("kmp", test_kmp); }
