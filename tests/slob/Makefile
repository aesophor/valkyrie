# Host-side stress test for the slob allocator (mm/SlobAllocator.cc).
#
#   make            build and run with seeds 1..$(SEEDS)
#   make SEEDS=100  run more seeds
#   make build      build only, then: ./build/slobtest <seed>
#
# The kernel sources are compiled on the host: their kernel includes are
# rewritten to stubs/common.h, which provides host versions of the bits
# the allocator needs (Kernel::panic, Page, BuddyAllocator, ...).

ROOT = ../..
BUILD_DIR = build
GEN_DIR = $(BUILD_DIR)/gen
SEEDS ?= 20

CXX = clang++
CXXFLAGS = -std=c++20 -g -O1 -fsanitize=address,undefined \
           -Dprivate=public -iquote stubs -I $(GEN_DIR)

# Kernel headers that are replaced by stubs/common.h.
STUBBED_INCLUDES = Algorithm|Math|String|TypeTraits|dev/Console|kernel/Kernel|mm/Page
REWRITE = sed -E 's@^\#include <($(STUBBED_INCLUDES))\.h>@\#include "common.h"@'

.PHONY: all build test clean

all: test

# Always rebuild: it only takes a few seconds, and make's mtime checks can
# miss a source change made within the same second (e.g. GNU make 3.81).
build:
	mkdir -p $(GEN_DIR)/mm
	$(REWRITE) $(ROOT)/include/mm/SlobAllocator.h > $(GEN_DIR)/mm/SlobAllocator.h
	$(REWRITE) $(ROOT)/mm/SlobAllocator.cc > $(GEN_DIR)/SlobAllocator.cc
	$(CXX) $(CXXFLAGS) test.cc $(GEN_DIR)/SlobAllocator.cc -o $(BUILD_DIR)/slobtest

test: build
	@for s in $$(seq 1 $(SEEDS)); do ./$(BUILD_DIR)/slobtest $$s || exit 1; done

clean:
	rm -rf $(BUILD_DIR)
