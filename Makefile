GCC_BASE=$(PROJ_ROOT)/output/gcc/bin

CC := $(GCC_BASE)/riscv32-rivai-elf-gcc
OBJDUMP := $(GCC_BASE)/riscv32-rivai-elf-objdump
READELF := $(GCC_BASE)/riscv32-rivai-elf-readelf

TARGET ?= sample.elf
COSIM_FLAG ?=

OUT_PATH ?= .

SRC_PATH := ./utils
SRC_PATH += ./

OPT_INCLUDE := -I. -I./utils
OPT_INCLUDE += $(INCLUDE)

DEFINES :=
CFLAGS := -Wall $(OPT_INCLUDE) $(DEFINES) $(COSIM_FLAG)
CFLAGS += -mcmodel=medany -O3 -mcpu=rugrats -mtune=rugrats_conure -march=rv32imafcv -fno-var-tracking-assignments
LDFLAGS := -T ./linker.ld -nostartfiles

SRC_FILES := $(foreach path, $(SRC_PATH), $(wildcard $(path)/*.c))

OBJ_FILES := $(patsubst %.c, %.o, $(SRC_FILES))
START_FILES := ./boot.S

.PHONY : all prepare clean run

all : clean prepare $(TARGET)

prepare :
	@echo create $(OUT_PATH)
	mkdir -p $(OUT_PATH)

$(TARGET) : $(OBJ_FILES)
	@echo
	@echo *********************Start Building...*********************
	@echo $^
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(START_FILES) -lm -o $(OUT_PATH)/$@
	$(OBJDUMP) -xDS $(OUT_PATH)/$@ > $(OUT_PATH)/$@.dump
	$(READELF) -a $(OUT_PATH)/$@ > $(OUT_PATH)/$@.header
	@echo *********************End Building...***********************
	@echo

run:
	@echo
	@echo *********************Start running...*********************
	spike --isa=rv32IMAFCV ++set_orv32_rst_pc=80000000 ++load_pk ++$(TARGET)
	@echo *********************End running...***********************
	@echo

clean :
	@echo
	@echo *********************Start Cleaning...*********************
	-rm -f $(OUT_PATH)/$(TARGET).* $(OUT_PATH)/$(TARGET) $(OBJ_FILES)
	@echo *********************End Cleaning...***********************
	@echo

