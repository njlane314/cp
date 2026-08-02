CXX ?= c++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Werror
TSTDIR ?= ../tst
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
LICENSEDIR ?= $(PREFIX)/share/licenses/libcp

ifeq ($(origin CXX),default)
ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
CXX := /opt/homebrew/opt/llvm/bin/clang++
else ifneq ($(wildcard /usr/local/opt/llvm/bin/clang++),)
CXX := /usr/local/opt/llvm/bin/clang++
endif
endif

headers := contract compressor disjoint fenwick kmp modint recursive segment types utility
include_root := include
public_dir := $(include_root)/cp
header_paths := $(headers) $(addprefix src/,$(addsuffix .hpp,$(headers)))
public_headers := $(addprefix $(public_dir)/,$(headers))
source_headers := $(addprefix $(public_dir)/src/,$(addsuffix .hpp,$(headers)))
all_headers := $(public_headers) $(source_headers)
build := .build
test_build := $(build)/tests
tst_header := $(TSTDIR)/tst.hpp
test_bins := $(addprefix $(test_build)/,$(headers))
release_test := $(test_build)/contract_release
tests := $(test_bins) $(release_test)

.DEFAULT_GOAL := check
.PHONY: check install clean

check: $(tests)
	@for header in $(header_paths); do \
		printf '#include <cp/%s>\nint main() {}\n' "$$header" | \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(include_root) -x c++ -fsyntax-only - || exit; \
	done
	@if printf '#include <cp/contract>\nint main() { CP_EXPECT(true, 42); }\n' | \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(include_root) -x c++ -fsyntax-only - \
		>$(build)/contract-type.log 2>&1; then \
		exit 1; \
	fi
	@for test_binary in $(tests); do "$$test_binary" || exit; done
	@set -e; for test_case in \
		'contract|failure|contract test failure' \
		'disjoint|negative-size|disjoint_set: negative size' \
		'fenwick|invalid-index|fenwick_tree: invalid position' \
		'segment|invalid-range|segment_tree::fold: invalid range' \
		'compressor|missing-rank|coordinate_compressor::rank: value is absent' \
		'compressor|invalid-value|coordinate_compressor::value: invalid position' \
		'compressor|oversized-input|coordinate_compressor: input is too large'; do \
		header=$${test_case%%|*}; remainder=$${test_case#*|}; \
		mode=$${remainder%%|*}; message=$${remainder#*|}; \
		output=$(build)/contract-$$header-$$mode.log; \
		if (ulimit -c 0; $(test_build)/$$header "$$mode") >"$$output" 2>&1; then exit 1; fi; \
		grep -Fq "cp: $$message" "$$output"; grep -Fq '  expected: ' "$$output"; \
		grep -Fq '  at: ' "$$output"; \
	done
	@rm -rf "$(build)/install"
	@$(MAKE) --no-print-directory install DESTDIR="$(CURDIR)/$(build)/install" PREFIX=/usr/local
	@for header in $(headers); do \
		printf '#include <cp/%s>\nint main() {}\n' "$$header" | \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I"$(build)/install/usr/local/include" \
		-x c++ -fsyntax-only - || exit; \
	done

$(test_bins): $(test_build)/%: test/%.cpp $(all_headers) $(tst_header) | $(test_build)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DLOCAL -I$(include_root) -I"$(TSTDIR)" $< -o $@

$(release_test): test/contract_release.cpp $(all_headers) $(tst_header) | $(test_build)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -ULOCAL -I$(include_root) -I"$(TSTDIR)" $< -o $@

$(test_build):
	mkdir -p $@

install:
	install -d "$(DESTDIR)$(INCLUDEDIR)/cp/src" "$(DESTDIR)$(LICENSEDIR)"
	install -m 0644 $(public_headers) "$(DESTDIR)$(INCLUDEDIR)/cp"
	install -m 0644 $(source_headers) "$(DESTDIR)$(INCLUDEDIR)/cp/src"
	install -m 0644 LICENSE "$(DESTDIR)$(LICENSEDIR)/LICENSE"

clean:
	rm -rf $(build)
