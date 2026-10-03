# Version 4 

.PHONY: all labs clean all-dockers
.SECONDEXPANSION:
.SECONDARY:

BOOST_LOCATION := $(shell test -f .boost_location && cat .boost_location ; true)
DOCKER_IMAGE ?= caseyrgb/rgb-tested:clang20

ifneq 'yes' '$(VERBOSE)'
hidecmd := @
endif
# The variable SILENT controls additional messages

CPPFLAGS += -Wall -Wextra -Werror -Wno-missing-field-initializers -Werror=vla -Wold-style-cast $(if $(BOOST_LOCATION),-isystem $(BOOST_LOCATION))
CXXFLAGS += -g

system   := $(shell uname)

ifneq 'MINGW' '$(patsubst MINGW%,MINGW,$(system))'
CPPFLAGS += -std=c++14
else
CPPFLAGS += -std=gnu++14
endif

ZIP_CMD := zip
ifeq 'Darwin' '$(system)'
TIMEOUT_CMD := gtimeout
else
TIMEOUT_CMD := timeout
endif

CLANG_FORMAT_MIN := 20
CLANG_TIDY_MIN := 20

students := $(filter-out out Makefile README.md,$(wildcard *))
labs     := $(foreach student,$(students),$(wildcard $(student)/??) $(wildcard $(student)/??.?))

student            = $(word 1,$(subst /, ,$(1)))

lab_test_sources   = $(wildcard $(1)/test-*.cpp)
lab_sources        = $(filter-out $(1)/test-%,$(wildcard $(1)/*.cpp))
lab_headers        = $(wildcard $(1)/*.h) $(wildcard $(1)/*.hpp) $(wildcard $(1)/*.hxx)
lab_common_sources = $(if $(wildcard $(1)/common),$(filter-out $(1)/common/test-%.cpp,$(wildcard $(1)/common/*.cpp)))
lab_common_tests   = $(if $(wildcard $(1)/common),$(wildcard $(1)/common/test-*.cpp))
lab_common_headers = $(if $(wildcard $(1)/common),$(wildcard $(1)/common/*.h) $(wildcard $(1)/common/*.hpp) $(wildcard $(1)/common/*.hxx))

lab_objects        = $(patsubst %.cpp,out/%.o,$(call lab_sources,$(1)) $(call lab_common_sources,$(call student,$(1))))
lab_test_objects   = $(patsubst %.cpp,out/%.o,$(call lab_test_sources,$(1)) $(call lab_common_tests,$(call student,$(1))))
lab_header_checks  = $(addprefix out/,$(addsuffix .header,$(call lab_headers,$(1)) $(call lab_common_headers,$(call student,$(1)))))

objects           := $(sort $(foreach lab,$(labs),$(call lab_objects,$(lab))))
test_objects      := $(sort $(foreach lab,$(labs),$(call lab_test_objects,$(lab))))
header_checks     := $(sort $(foreach lab,$(labs),$(call lab_header_checks,$(lab))))

common_include     = $(if $(wildcard $(call student,$(1))/common),-I$(call student,$(1))/common -I$(call student,$(1))/common/include)

all: $(addprefix build-,$(labs))

labs:
	@echo $(labs)

all-dockers: $(addprefix docker-test-,$(labs))

$(addprefix run-,$(labs)): run-%: out/%/lab
	@$(FAULT_INJECTION_CONFIG) $(if $(TIMEOUT),$(TIMEOUT_CMD) --signal=KILL $(TIMEOUT)s )$(if $(VALGRIND),valgrind $(VALGRIND) )$< $(ARGS)

clean:
	rm -rf out

$(addprefix build-,$(labs)): build-%: out/%/lab

$(addprefix zip-,$(labs)): zip-%: out/%/src-lab

$(addprefix test-,$(labs)): test-%: out/%/test-lab
	$(if $(SILENT),,@echo [TEST] $(patsubst out/%/test-lab,%,$<))
	$(hidecmd)$(if $(TIMEOUT),$(TIMEOUT_CMD) --signal=KILL $(TIMEOUT)s )$(if $(VALGRIND),valgrind $(VALGRIND) )$< $(TEST_ARGS)

out/%/src-lab: Makefile $$(call lab_sources,%) $$(call lab_headers,%) $$(call lab_common_sources,$$(call student,%)) $$(call lab_common_headers,$$(call student,%)) | $$(@D)/.dir
	$(if $(SILENT),,@echo [ZIP ] $(patsubst out/%/lab-src,%,$@))
	$(hidecmd)$(ZIP_CMD) -r $@ $^

out/%/lab: $$(call lab_objects,%) $$(call lab_header_checks,%) | $$(@D)/.dir
	$(if $(SILENT),,@echo [LINK] $(patsubst out/%/lab,%,$@))
	$(hidecmd)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter-out %.header,$^)

out/%/test-lab: $$(call lab_test_objects,%) $$(call lab_objects,%) | $$(@D)/.dir
	$(if $(SILENT),,@echo [LINK] $(patsubst out/%/test-lab,%,$@))
	$(hidecmd)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter-out %/main.o,$^)

$(test_objects): out/%.o: %.cpp | $$(@D)/.dir
	$(if $(SILENT),,@echo [C++ ] $<)
	$(hidecmd)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast -Wno-unused-parameter -MMD -MP -c $(call common_include,$<) -o $@ $<

$(objects): out/%.o: %.cpp | $$(@D)/.dir
	$(if $(SILENT),,@echo [C++ ] $<)
	$(hidecmd)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $(call common_include,$<) -o $@ $<

$(header_checks): out/%.header: % | $$(@D)/.dir
	$(if $(SILENT),,@echo [HDR ] $<)
	$(hidecmd)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-unused-const-variable -c $(call common_include,$<) -fsyntax-only $<
	@touch $@

%/.dir:
	@mkdir -p $(@D) && touch $@

check-docker:
	@which docker > /dev/null || (echo "Docker not installed. Run (Ubuntu/Debian):\nsudo apt install docker.io" && exit 1)
	@docker version > /dev/null 2>&1 || (echo "Docker permission denied. Run:\nsudo usermod -aG docker $$USER\nThen re-login" && exit 1)

$(addprefix doctest-,$(labs)): doctest-%: check-docker
	$(eval student := $(word 1,$(subst /, ,$*)))
	$(eval lab     := $(notdir $*))

	@docker run --rm \
		-v $(PWD):/workspace \
		-w /workspace \
		-e LAB=$(lab) \
		-e STUDENT=$(student) \
		-e BASE_BRANCH=origin/master \
		$(DOCKER_IMAGE) \
		/bin/bash -c "\
			cd /workspace && \
			echo '===   build    ===' && \
			make build-$(student)/$(lab) && \
			echo '=== acceptance ===' && \
			/spbspu-labs-tests/test-lab-$(lab) $(student) out/$(student)/$(lab)/acceptance.xml || true && \
			sleep 2s && \
			echo '===  results   ===' && \
			xsltproc -o 'out/$(student)/$(lab)/acceptance.md' \
			'/spbspu-labs-tests/report-md.xslt' \
			'out/$(student)/$(lab)/acceptance.xml' && \
			cat out/$(student)/$(lab)/acceptance.md"

	@rm -f vgcore.*

$(addprefix doclint-,$(labs)): doclint-%: check-docker
	@docker run --rm \
		-v $(PWD):/workspace \
		-w /workspace \
		-e BASE_BRANCH=origin/master \
		$(DOCKER_IMAGE) \
		/bin/bash -c "\
			cd /workspace && \
			echo '=== CG ===' && \
			/spbspu-labs-tests/validate-cg.sh"

	@rm -f vgcore.*

check-clang-format:
	$(eval LATEST_CLANG_FORMAT := $(shell \
    V=$$(clang-format --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1 | cut -d. -f1); \
    if [ -n "$$V" ] && [ $$V -ge $(CLANG_FORMAT_MIN) ]; then \
      echo "clang-format"; \
    else \
      find /usr/bin /bin -name "clang-format*" -printf "%f\n" 2>/dev/null | \
        grep -E "clang-format-[0-9]+$$" | \
        sort -V | \
        awk -F- '$$NF >= $(CLANG_FORMAT_MIN)' | \
        tail -n1; \
    fi))
		
	@if [ "$(LATEST_CLANG_FORMAT)" = "" ]; then \
		echo "[FORMAT] clang-format not installed or have version under $(CLANG_FORMAT_MIN)"; \
		exit 1; \
	fi

$(addprefix format-,$(labs)): format-%: check-clang-format
	$(eval check := $(if $(filter --check,$(ARGS)),1,$(if $(ARGS),0,1)))
	$(eval fix := $(if $(filter --fix,$(ARGS)),1,0))
	$(eval format_general := .github/cg/linters/format/.clang-format)
	$(eval format_break := .github/cg/linters/format/format-break/.clang-format)
	$(eval format_egypt := .github/cg/linters/format/format-egypt/.clang-format)
	$(eval format_write := ./.clang-format)

	@if [ $(check) = 0 ] && [ $(fix) = 0 ]; then \
		python3 .github/cg/scripts/info.py; \
	fi

	$(eval files_sources := $(call lab_sources,$*))
	$(eval files_headers := $(call lab_headers,$*))
	$(eval files_tests := $(call lab_test_sources,$*))
	$(eval files_common_sources := $(call lab_common_sources,$(call student,$*)))
	$(eval files_common_headers := $(call lab__common_headers,$(call student,$*)))
	$(eval files_common_tests := $(call lab_common_tests,$(call student,$*)))
	$(eval files_all := $(files_sources) $(files_headers) $(files_tests) \
		$(files_common_sources) $(files_common_headers) $(files_common_tests))

	@if [ $(check) = 1 ] || [ $(fix) = 1 ]; then \
		touch .clang-format; \
		python3 .github/cg/scripts/format/complete-format-with-parentheses.py \
			$(format_write) $(format_general) $(format_break) $(format_egypt) $(files_all); \
	fi

	@if [ $(check) = 1 ]; then \
		echo "[FORMAT] check files"; \
		python3 .github/cg/scripts/format/check-comments.py "--check" $(files_all); \
		python3 .github/cg/scripts/format/check-std-spaces.py "--check" $(files_all); \
		python3 .github/cg/scripts/format/check-alias-length.py "--check" $(files_all); \
		$(LATEST_CLANG_FORMAT) -style=file:.clang-format -n -Werror $(files_all) || true; \
	fi
	@if [ $(fix) = 1 ]; then \
		echo "[FORMAT] fix files"; \
		python3 .github/cg/scripts/format/check-comments.py "--fix" $(files_all); \
		python3 .github/cg/scripts/format/check-std-spaces.py "--fix" $(files_all); \
		$(LATEST_CLANG_FORMAT) -style=file:.clang-format -i -Werror $(files_all) || true; \
	fi

	@rm -f .clang-format

check-clang-tidy:
	$(eval LATEST_CLANG_TIDY := $(shell \
    V=$$(clang-tidy --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1 | cut -d. -f1); \
    if [ -n "$$V" ] && [ $$V -ge $(CLANG_TIDY_MIN) ]; then \
      echo "clang-tidy"; \
    else \
      find /usr/bin /bin -name "clang-tidy*" -printf "%f\n" 2>/dev/null | \
        grep -E "clang-tidy-[0-9]+$$" | \
        sort -V | \
        awk -F- '$$NF >= $(CLANG_TIDY_MIN)' | \
        tail -n1; \
    fi))
		
	@if [ "$(LATEST_CLANG_TIDY)" = "" ]; then \
		echo "[TIDY] clang-tidy not installed or have version under $(CLANG_TIDY_MIN)"; \
		exit 1; \
	fi

$(addprefix tidy-,$(labs)): tidy-%: check-clang-tidy
	$(eval check := $(if $(filter --check,$(ARGS)),1,$(if $(ARGS),0,1)))
	$(eval tidy_general := .github/cg/linters/tidy/.clang-tidy)
	$(eval tidy_camel := .github/cg/linters/tidy/tidy-camel/.clang-tidy)
	$(eval tidy_lower := .github/cg/linters/tidy/tidy-lower/.clang-tidy)
	$(eval tidy_write := ./.clang-tidy)

	@if [ $(check) = 0 ]; then \
		python3 .github/cg/scripts/info.py; \
	fi

	$(eval files_sources := $(call lab_sources,$*))
	$(eval files_headers := $(call lab_headers,$*))
	$(eval files_tests := $(call lab_test_sources,$*))
	$(eval files_common_sources := $(call lab_common_sources,$(call student,$*)))
	$(eval files_common_headers := $(call lab__common_headers,$(call student,$*)))
	$(eval files_common_tests := $(call lab_common_tests,$(call student,$*)))
	$(eval files_all := $(files_sources) $(files_headers) $(files_tests) \
		$(files_common_sources) $(files_common_headers) $(files_common_tests))

	@if [ $(check) = 1 ]; then \
		touch .clang-tidy; \
		python3 .github/cg/scripts/tidy/complete-tidy-with-case.py \
			$(tidy_write) $(tidy_general) $(tidy_camel) $(tidy_lower) $(files_all); \
		echo "[TIDY] check files"; \
		$(LATEST_CLANG_TIDY) --config-file=.clang-tidy -header-filter='.*' --warnings-as-errors='*' --quiet $(files_all); \
		python3 .github/cg/scripts/tidy/check-header-guard.py "--check" $(files_all) || true; \
	fi

	@rm -f .clang-tidy

include $(wildcard $(patsubst %.o,%.d,$(objects) $(test_objects)))
