#### Globals
BUILD_DIR = build
# For local development enable in-place file changes
FMT_MESON = -i
FMT_CLANG_FORMAT = -i

#### Macros
# Macro that generates `<component>` and `<component>-run` targets
# $1 = make target name (e.g. front)
# $2 = relative path from the root of the project down to exe
define GENERATE_TARGET_BUILD_AND_RUN
$1: .ensure-builddir-exists
	@meson compile -C $(BUILD_DIR) $(notdir $2)

$1-run: $1
	@./$2
endef

#### Building main targets
$(eval $(call GENERATE_TARGET_BUILD_AND_RUN,front,$(BUILD_DIR)/bins/FrontEnd/front_end_exe))
$(eval $(call GENERATE_TARGET_BUILD_AND_RUN,middle,$(BUILD_DIR)/bins/MiddleEnd/middle_end_exe))
$(eval $(call GENERATE_TARGET_BUILD_AND_RUN,reverse,$(BUILD_DIR)/bins/ReverseEnd/reverse_end_exe))
$(eval $(call GENERATE_TARGET_BUILD_AND_RUN,tests,$(BUILD_DIR)/tests/tests_exe))
all: front middle reverse tests

# because there are a lot of sanitizer errors out there :D
tests-run-ignore-sanitizer-output:
	$(MAKE) tests-run 2>/dev/null

#### Build management
clean:
	@meson compile -C $(BUILD_DIR) --clean 2>/dev/null || rm -rf $(BUILD_DIR)

# 'rebuild' should use Meson's own reconfigure if possible, it's faster than rm -rf
soft-rebuild:
	@meson setup --reconfigure $(BUILD_DIR)

# `hard-rebuild` is useful in case of builddir corruption as it wipes the builddir and reconfigures it
hard-rebuild:
	@meson setup --wipe $(BUILD_DIR)

.ensure-builddir-exists:
	@[ -d $(BUILD_DIR) ] || meson setup $(BUILD_DIR)

#### Pretty
fmt:
	@meson format -r $(FMT_MESON)
	@find bins libs tests -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --Werror $(FMT_CLANG_FORMAT)

lint: $(BUILD_DIR)/compile_commands.json
	@run-clang-tidy -quiet -p $(BUILD_DIR) bins libs tests

lint-fix: $(BUILD_DIR)/compile_commands.json
	@run-clang-tidy -quiet -fix -p $(BUILD_DIR) bins libs tests

#### CI check
check: FMT_MESON = --check-only
check: FMT_CLANG_FORMAT = --dry-run
check: fmt lint


#### Misc
list:
	@echo "Available targets:"
	@echo "  all                                  - Build everything"
	@echo "  (front|middle|reverse|tests)[-run]   - Build component [and run it]"
	@echo "  fmt                                  - Format code using clang-format"
	@echo "  lint                                 - Lint code using clang-tidy"
	@echo "  lint-fix                             - Lint code using clang-tidy and auto-fix some errors"
	@echo "  rebuild                              - Wipe and regenerate build directory"
	@echo "  clean                                - Delete build directory"

.PHONY: all clean rebuild list fmt lint front middle reverse tests front-run middle-run reverse-run tests-run
