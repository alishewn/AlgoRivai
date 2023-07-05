######################################################################
CROSS_COMPILER_ROOT   = $(PROJ_ROOT)/output/gcc/bin
CROSS_COMPILER_PREFIX = riscv32-rivai-elf-
CROSS_COMPILER        = $(CROSS_COMPILER_ROOT)/$(CROSS_COMPILER_PREFIX)
######################################################################

######################################################################
CC      = $(CROSS_COMPILER)gcc
AS      = $(CROSS_COMPILER)gcc -x assembler-with-cpp
OD      = $(CROSS_COMPILER)objdump -xDS
OC      = $(CROSS_COMPILER)objcopy
AR      = $(CROSS_COMPILER)ar
SZ      = $(CROSS_COMPILER)size
RE = $(CROSS_COMPILER)readelf -a
HEX = $(OC) -O ihex
BIN = $(OC) -O binary -S
######################################################################

######################################################################
BUILD_DIR = build
OUT_DIR   = output
MKDIR_P  ?= mkdir -p
target    = alishewn
debug     = y
#####################################################################

#####################################################################
INC         = ./ ./utils
INCDIRS    := $(addprefix -I, $(INC))

C_SOURCES   = $(shell find ./ -name '*.c')
ASM_SOURCES = ./boot.S

FPU         =
CPU_FLAGS   = -mcmodel=medany -mcpu=rugrats -mtune=rugrats_conure -march=rv32imafcv 
AS_DEFS     =

MACROS_STR +=
MACROS     := $(addprefix -D, $(DEFS_STR))

CFLAGS      = $(CPU_FLAGS) $(MACROS) $(INCDIRS) -Wall  -Wfatal-errors -MMD -fdata-sections -ffunction-sections
ifeq ($(debug), y)
    CFLAGS += -g -O0
else
    CFLAGS += -O3
endif

ASFLAGS     = $(CFLAGS) $(AS_DEFS)
#####################################################################


#####################################################################
LDSCRIPT = ./linker.ld

LIBS    = -lc -lm
LIBDIR  =
LDFLAGS = $(CPU_FLAGS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) \
          -Wl,-Map=$(BUILD_DIR)/$(target).map,--cref -Wl,--gc-sections -nostartfiles

ifeq ($(V),1)
Q=
NQ=true
else
Q=@
NQ=echo
endif

all: $(BUILD_DIR)/$(target).elf $(BUILD_DIR)/$(target).objdump $(BUILD_DIR)/$(target).header $(BUILD_DIR)/$(target).hex $(BUILD_DIR)/$(target).bin
#####################################################################

#####################################################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

DEPS := $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@$(NQ) "Compiling: " $(basename $(notdir $@)).c
	$(Q)$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@$(NQ) "Compiling: " $(basename $(notdir $@)).S
	$(Q)$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(target).elf: $(OBJECTS) Makefile | $(OUT_DIR)
	@$(NQ) "Generating elf file..." $(notdir $@)
	$(Q)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.objdump : $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUT_DIR)
	@$(NQ) "Generating objdump file..." $(notdir $@)
	$(Q)$(OD) $< > $@
	$(Q)mv $@ $(OUT_DIR)

$(BUILD_DIR)/%.header : $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUT_DIR)
	@$(NQ) "Generating objdump file..." $(notdir $@)
	$(Q)$(RE) $< > $@
	$(Q)mv $@ $(OUT_DIR)

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUT_DIR)
	@$(NQ) "Generating hex file..." $(notdir $@)
	$(Q)$(HEX) $< $@
	$(Q)mv $@ $(OUT_DIR)

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUT_DIR)
	$(NQ) "Generating bin file..." $(notdir $@)
	$(Q)$(BIN) $< $@
	$(Q)mv $@ $(OUT_DIR)
	$(Q)mv $< $(OUT_DIR)

$(BUILD_DIR):
	mkdir $@

$(OUT_DIR):
	mkdir $@

clean:
	-rm -fR .dep $(BUILD_DIR) $(OUT_DIR)
	@find . -iname '*.o' -o -iname '*.bak' -o -iname '*.d' | xargs rm -f

run:
	@echo
	@echo *********************Start running...*********************
	spike --isa=rv32IMAFCV ++set_orv32_rst_pc=80000000 ++load_pk ++$(OUT_DIR)/$(target).elf
	@echo *********************Stop  running...*********************
	@echo

.PHONY: all clean
