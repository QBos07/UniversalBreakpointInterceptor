LIBSOURCEDIR = lib
EXESOURCEDIR = example
INCLUDEDIR = include
BUILDDIR = obj
OUTDIR = dist
DEPDIR = .deps

AS:=sh4a_nofpueb-elf-gcc
AS_FLAGS:=-gdwarf-5

SDK_DIR?=/sdk

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
WARNINGS=-Wall -Wextra -pedantic -Werror -pedantic-errors
INCLUDES=-I$(SDK_DIR)/include -I$(INCLUDEDIR)
DEFINES=
FUNCTION_FLAGS=-ffunction-sections -fdata-sections -gdwarf-5 -O2
COMMON_FLAGS=$(FUNCTION_FLAGS) $(INCLUDES) $(WARNINGS) $(DEFINES)

CC:=sh4a_nofpueb-elf-gcc
CC_FLAGS=-std=c23 $(COMMON_FLAGS)

CXX:=sh4a_nofpueb-elf-g++
CXX_FLAGS=-std=c++23 $(COMMON_FLAGS)

LD:=sh4a_nofpueb-elf-g++
LD_FLAGS:=$(FUNCTION_FLAGS) -Wl,--gc-sections -Wl,-Ttext-segment,0x8C052800
LIBS:=-L$(SDK_DIR) -lsdk

AR:=sh4a_nofpueb-elf-ar
AR_FLAGS:=src

READELF:=sh4a_nofpueb-elf-readelf
OBJCOPY:=sh4a_nofpueb-elf-objcopy
STRIP:=sh4a_nofpueb-elf-strip

LIB_ARCHIVE := $(OUTDIR)/libubi.a
APP_ELF := $(OUTDIR)/UBITest.elf
APP_HH3 := $(APP_ELF:.elf=.hh3)

LIB_AS_SOURCES:=$(shell find $(LIBSOURCEDIR) -name '*.S')
LIB_CC_SOURCES:=$(shell find $(LIBSOURCEDIR) -name '*.c')
LIB_CXX_SOURCES:=$(shell find $(LIBSOURCEDIR) -name '*.cpp')
EXE_AS_SOURCES:=$(shell find $(EXESOURCEDIR) -name '*.S')
EXE_CC_SOURCES:=$(shell find $(EXESOURCEDIR) -name '*.c')
EXE_CXX_SOURCES:=$(shell find $(EXESOURCEDIR) -name '*.cpp')
LIB_OBJECTS := $(addprefix $(BUILDDIR)/,$(LIB_AS_SOURCES:.S=.o)) \
	$(addprefix $(BUILDDIR)/,$(LIB_CC_SOURCES:.c=.o)) \
	$(addprefix $(BUILDDIR)/,$(LIB_CXX_SOURCES:.cpp=.o))
EXE_OBJECTS := $(addprefix $(BUILDDIR)/,$(EXE_AS_SOURCES:.S=.o)) \
	$(addprefix $(BUILDDIR)/,$(EXE_CC_SOURCES:.c=.o)) \
	$(addprefix $(BUILDDIR)/,$(EXE_CXX_SOURCES:.cpp=.o))

DEPFILES := $(OBJECTS:$(BUILDDIR)/%.o=$(DEPDIR)/%.d)

hh3: $(APP_HH3) Makefile
elf: $(APP_ELF) Makefile

all: lib elf hh3
.DEFAULT_GOAL := all
.SECONDARY: # Prevents intermediate files from being deleted

.NOTPARALLEL: clean
clean:
	rm -rf $(BUILDDIR) $(OUTDIR) $(DEPDIR)

%.hh3: %.elf
	$(STRIP) -o $@ $^

$(APP_ELF): $(EXE_OBJECTS) $(LIB_ARCHIVE)
	@mkdir -p $(dir $@)
	$(LD) -Wl,-Map $@.map -o $@ $(LD_FLAGS) $^ $(LIBS)

$(LIB_ARCHIVE): $(LIB_OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) $(AR_FLAGS) $@ $^

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) -c $< -o $@ $(AS_FLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEPDIR)/$<)
	+$(CC) -c $< -o $@ $(CC_FLAGS) $(DEPFLAGS)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEPDIR)/$<)
	+$(CXX) -c $< -o $@ $(CXX_FLAGS) $(DEPFLAGS)

compile_commands.json:
	$(MAKE) clean
	bear -- sh -c "$(MAKE) --keep-going all || exit 0"

.PHONY: elf hh3 all clean compile_commands.json

-include $(DEPFILES)