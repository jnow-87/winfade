################
###   init   ###
################

# init build system variables
project_type := c
scripts_dir := scripts/build
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Pconfig
githooks_tree := .githooks
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/$(CONFIG_BUILD_TYPE)/
src_dirs := fade/ do/ mousemv/

# include build system Makefile
include $(scripts_dir)/main.make

# init default flags
cflags := \
	$(CFLAGS) \
	$(CONFIG_CFLAGS) \
	-O2 \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-stringop-truncation

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	-Iinclude \
	-I$(src_tree) \
	-I$(build_tree)

ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

yaccflags := $(YACCFLAGS) $(CONFIG_YACCFLAGS)
lexflags := $(LEXFLAGS) $(CONFIG_LEXFLAGS)
gperfflags := $(GPERFFLAGS) $(CONFIG_GPERFFLAGS)

###################
###   targets   ###
###################

####
## build
####
.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g
all: cxxflags += -g
all: asflags += -g
endif

all: $(lib) $(bin)

####
## cleanup
####
.PHONY: clean
clean:
	$(rm) $(filter-out $(patsubst %/,%,$(dir $(build_tree)/$(scripts_dir))),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(config).old .clang $(build_tree)

####
## install
####
.PHONY: install-user
install-user: all

.PHONY: install-system
install-system: all

.PHONY: uninstall
uninstall:

####
## help
####

.PHONY: help
help:
