BUILD_DIR = build
TARGETS   = front_end_exe middle_end_exe reverse_end_exe

front:   $(BUILD_DIR)/bins/FrontEnd/front_end_exe
middle:  $(BUILD_DIR)/bins/MiddleEnd/middle_end_exe
reverse: $(BUILD_DIR)/bins/ReverseEnd/reverse_end_exe
tests:   $(BUILD_DIR)/tests/tests_exe
all: 		 front middle reverse tests

# MACRO: $1 = target name (e.g. front), $2 = path to exe
define GEN_RULES
$1-run: $2
	./$2

# This allows 'make' to track the file. 
# We depend on build/build.ninja so we don't 'meson setup' unnecessarily.
$2: $(BUILD_DIR)/build.ninja
	@meson compile -C $(BUILD_DIR) $$(notdir $2)
endef

$(eval $(call GEN_RULES,front,$(BUILD_DIR)/bins/FrontEnd/front_end_exe))
$(eval $(call GEN_RULES,middle,$(BUILD_DIR)/bins/MiddleEnd/middle_end_exe))
$(eval $(call GEN_RULES,reverse,$(BUILD_DIR)/bins/ReverseEnd/reverse_end_exe))
$(eval $(call GEN_RULES,tests,$(BUILD_DIR)/tests/tests_exe))

# Ensure the build directory is initialized
$(BUILD_DIR)/build.ninja:
	@if [ ! -d $(BUILD_DIR) ]; then \
		meson setup $(BUILD_DIR); \
	else \
		echo "Build dir exists but is uninitialized. Running setup..."; \
		meson setup --reconfigure $(BUILD_DIR); \
	fi

$(BUILD_DIR)/compile_commands.json: $(BUILD_DIR)/build.ninja

clean:
	@meson compile -C $(BUILD_DIR) --clean 2>/dev/null || rm -rf $(BUILD_DIR)

# 'rebuild' should use Meson's own reconfigure if possible, it's faster than rm -rf
rebuild:
	@meson setup --wipe $(BUILD_DIR)
	@$(MAKE) all

# For local development enable in-place file changes
FMT_MESON = -i
FMT_CLANG_FORMAT = -i

fmt:
	@meson format -r $(FMT_MESON)
	@find bins libs tests -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --Werror $(FMT_CLANG_FORMAT)

lint: $(BUILD_DIR)/compile_commands.json
	@run-clang-tidy -quiet -p $(BUILD_DIR) bins libs

# For CI, checking without changing files
check: FMT_MESON = --check-only
check: FMT_CLANG_FORMAT = --dry-run
check: fmt lint

list:
	@echo "Available targets:"
	@echo "  all                                  - Build everything"
	@echo "  (front|middle|reverse|tests)[-run]   - Build component [and run it]"
	@echo "  fmt                                  - Format code using clang-format"
	@echo "  lint                                 - Lint code using clang-tidy"
	@echo "  rebuild                              - Wipe and regenerate build directory"
	@echo "  clean                                - Delete build directory"

.PHONY: all clean rebuild list fmt lint front middle reverse tests front-run middle-run reverse-run tests-run
