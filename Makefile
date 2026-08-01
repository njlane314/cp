CXX ?= c++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Werror
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
LICENSEDIR ?= $(PREFIX)/share/licenses/cp

ifeq ($(origin CXX),default)
ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
CXX := /opt/homebrew/opt/llvm/bin/clang++
else ifneq ($(wildcard /usr/local/opt/llvm/bin/clang++),)
CXX := /usr/local/opt/llvm/bin/clang++
endif
endif

headers := contract disjoint_set fenwick_tree kmp modint segment_tree types utility
build := .build
include_dir := $(build)/include
tests := $(build)/test $(build)/release

.DEFAULT_GOAL := check
.PHONY: check install clean

check: $(tests)
	@for header in $(headers); do \
		printf '#include <cp/%s>\nint main() {}\n' "$$header" | \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(include_dir) -x c++ -fsyntax-only - || exit; \
	done
	@$(build)/test
	@$(build)/release
	@set -e; for test_case in \
		'disjoint-size|disjoint_set: negative size' \
		'fenwick-index|fenwick_tree: invalid position' \
		'segment-range|segment_tree: invalid range' \
		'modint-inverse|modint: value has no multiplicative inverse'; do \
		mode=$${test_case%%|*}; message=$${test_case#*|}; output=$(build)/contract-$$mode.log; \
		if (ulimit -c 0; $(build)/test "$$mode") >"$$output" 2>&1; then exit 1; fi; \
		grep -Fq "cp: $$message" "$$output"; grep -Fq '  expected: ' "$$output"; \
		grep -Fq '  at: ' "$$output"; \
	done

$(tests): $(build)/%: test/%.cpp $(headers) | $(include_dir)/cp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(include_dir) $< -o $@

$(build)/test: test/test.cpp

$(include_dir)/cp:
	mkdir -p $(@D)
	ln -s ../.. $@

install:
	install -d "$(DESTDIR)$(INCLUDEDIR)/cp" "$(DESTDIR)$(LICENSEDIR)"
	install -m 0644 $(headers) "$(DESTDIR)$(INCLUDEDIR)/cp"
	install -m 0644 LICENSE "$(DESTDIR)$(LICENSEDIR)/LICENSE"

clean:
	rm -rf $(build)
