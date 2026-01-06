REPO_ROOT ?= ../..

include $(REPO_ROOT)/toolchain.mk
include $(REPO_ROOT)/sources.mk

BDIR ?= build/$(ARCH)

# These are the flags used to compile normal binaries throughout src/*.c
CPPFLAGS += -Iinclude -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -Wall -g -Wno-deprecated-declarations -Wno-parentheses \
	-Wno-stringop-overflow -Wno-sign-compare -Wno-format-security \
	-Wno-maybe-uninitialized \
	-I$(REPO_ROOT)/include -I$(BDIR) -I$(BDIR)/gen -I$(BDIR)/gen/src
LDFLAGS += $(LIBCRYPTO) $(LIBTSS2) $(LIBPROTOBUF) -lstdc++

PROGS:= $(BDIR)/rmasmoke
SRCS  = args.cc
SRCS += nvmem.cc
SRCS += rmasmoke.cc
SRCS += rop.cc
CXX_OBJS := $(addprefix $(BDIR)/, $(SRCS:.cc=.o))
OBJS := $(BDIR)/gen/src/tpm_manager.pb.cc $(CXX_OBJS:.c=.o) 
# Use VERBOSE=1 for debug output
ifeq ($(VERBOSE),)
Q := @
SUPPRESS := >/dev/null
else
Q :=
SUPPRESS := --verbose
endif

all: $(BDIR) $(PROGS)

.PHONY: clean
clean:
	$(Q)$(RM) -rf build

$(BDIR):
	$(Q)$(MKDIR) -p $@

$(BDIR)/version.h:
	$(Q)$(SHELL) $(REPO_ROOT)/util/version.sh $@


$(BDIR)/gen/src/tpm_manager.pb.cc: src/tpm_manager.proto
	@echo "  PROTOC  $<"
	$(Q)$(MKDIR) -p $(BDIR)/gen
	$(Q)$(PROTOC) $< --cpp_out=$(BDIR)/gen

$(BDIR)/%.o: src/%.c
	@echo "  CC      $(notdir $<)"
	$(Q)$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(BDIR)/%.o: src/%.cc
	@echo "  CXX     $(notdir $<)"
	$(Q)$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(PROGS): $(BDIR)/version.h $(OBJS)
	@echo "  LD      $(notdir $@)"
	$(Q)$(LD) $^ -o $@ $(CFLAGS) $(LDFLAGS)