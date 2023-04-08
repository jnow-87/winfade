#
# Copyright (C) 2014 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## init
####

# remove trailing '/'
loc_dir := $(patsubst %/,%,$(loc_dir))

# init local source and binary tree references, removing trailing '/' (in case $(loc_dir) is empty)
loc_src_tree := $(patsubst %/,%,$(src_tree)/$(loc_dir))
loc_build_tree := $(patsubst %/,%,$(build_tree)/$(loc_dir))

# copy subdir-*-*flags inherited from parent directory to loc_subdir-*flags
$(foreach flag,$(all_flags), \
	$(eval loc_subdir-$(flag) := $(subdir-$(loc_dir)-$(flag))) \
)

# debug message
$(call pdebug0)
$(call pdebug0,=== traverse into: src = $(loc_src_tree) - build = $(loc_build_tree) ===)
$(call pdebug0)

$(foreach flag,$(all_flags), \
	$(call pdebug1,    subdir $(flag): $(subdir-$(flag))) \
)
$(call pdebug0)

# init local variables
ext_dir :=

$(foreach type,$(all_types), \
	$(eval loc_single_$(type) :=) \
	$(eval loc_comp_$(type) :=) \
)

# init variables set by included makefiles
subdir-y :=

$(foreach type,$(all_types), \
	$(eval $(type)-y :=) \
)

$(foreach flag,$(all_flags), \
	$(eval $(flag)-y :=) \
	$(eval subdir-$(flag) :=) \
)


####
## include Makefile
####

# backup *flags
$(foreach flag,$(all_flags), \
	$(eval $(flag)_save := $($(flag))) \
)

include $(loc_src_tree)/Makefile

# check if *flags have been altered
$(foreach flag,$(all_flags), \
	$(if $(strip $(filter-out $($(flag)),$($(flag)_save))), \
		$(error $(loc_src_tree)/Makefile removed options from $(flag), changing from "$($(flag)_save)" to "$($(flag))", make sure to only add options), \
	) \
)


####
## filter input
####

$(call pdebug1,filter input)


## filter loc_single_*, loc_comp_*, loc_dir_*

# split $(type)_y into
#	loc_single_$(type): non-compound targets
#	loc_comp_$(type): compound targets
#	loc_dir_$(type): directories suffixed with obj.o
$(foreach type,$(all_types), \
	$(eval loc_dir_$(type) := $(addsuffix obj.o,$(filter %/,$($(type)-y)))) \
	\
	$(eval $(type)-y := $(filter-out %/,$($(type)-y))) \
	\
	$(eval loc_single_$(type) := $(call filter_single_dep,$($(type)-y))) \
	$(eval loc_comp_$(type) := $(filter-out $(loc_single_$(type)), $($(type)-y))) \
)

# .o -> .host.o for host-types, e.g. hostobj
$(foreach type,$(filter host%,$(all_types)), \
	$(eval loc_single_$(type) := $(patsubst %.o,%.host.o,$(loc_single_$(type)))) \
	$(eval loc_comp_$(type) := $(patsubst %.o,%.host.o,$(loc_comp_$(type)))) \
	$(eval loc_dir_$(type) := $(patsubst %.o,%.host.o,$(loc_dir_$(type)))) \
)

$(foreach type,$(all_types), \
	$(call pdebug1,    loc_single_$(type): $(loc_single_$(type))) \
	$(call pdebug1,    loc_comp_$(type): $(loc_comp_$(type))) \
	$(call pdebug1,    loc_dir_$(type): $(loc_dir_$(type))) \
	$(call pdebug1,) \
)


## process directories in targets

$(call pdebug1)
$(call pdebug1,handle directories)

# handle directories listed in $(obj_types)
#
#	- update list of sub-directories (subdir-y) and non-sub-directories (ext_dir)
#  	  select appropriate prefix by checking if the directory exists within $(loc_src_tree),
#  	  if not assume it exists in $(src_tree)
#
#	- postfix directories with obj.o e.g.
#		obj-y := <dir>/ -> obj-y := <dir>/obj.o
#
#		(obj.host.o is handled when generating rules, cf. gen_rule_comp)
#
#	- update $(loc_dir_*)
$(foreach type,$(obj_types), \
	$(eval deps :=) \
	\
	$(foreach obj,$(loc_dir_$(type)), \
		$(eval xdir := $(dir $(obj))) \
		\
		$(if $(call exists,$(loc_src_tree)/$(xdir)), \
			$(eval subdir-y += $(xdir)) \
			$(eval deps += $(loc_build_tree)/$(obj)) \
			, \
			$(eval ext_dir += $(xdir)) \
			$(eval deps += $(build_tree)/$(obj)) \
		) \
	) \
	\
	$(eval loc_dir_$(type) := $(deps)) \
)

# handle directories listed in compound-targets (<target>-y)
#
#	- update list of sub-directories (subdir-y) and non-sub-directories (ext_dir)
#  	  select appropriate prefix by checking if the directory exists within $(loc_src_tree),
#  	  if not assume it exists in $(src_tree)
#
#  	- postfix directories with obj.o (obj.host.o is handled when generating rules, cf. gen_rule_comp)
#  	- update the respective compound-target
$(foreach type,$(all_types), \
	$(foreach tgt,$(loc_comp_$(type)), \
		$(eval deps :=) \
		\
		$(foreach dir,$(filter %/,$($(call hostbasename,$(tgt))-y)), \
			$(if $(call exists,$(loc_src_tree)/$(dir)), \
				$(eval subdir-y += $(dir)) \
				$(eval deps += $(loc_build_tree)/$(dir)obj.o) \
				, \
				$(eval ext_dir += $(dir)) \
				$(eval deps += $(build_tree)/$(dir)obj.o) \
			) \
		) \
		\
		$(eval $(call hostbasename,$(tgt))-y := $(deps) $(filter-out %/,$($(call hostbasename,$(tgt))-y))) \
	) \
)

# remove duplicates
subdir-y := $(sort $(subdir-y))
ext_dir := $(sort $(ext_dir))

$(call pdebug1)
$(call pdebug1,    subdir: $(subdir-y))
$(call pdebug1,    external-dir: $(ext_dir))


## collect list of all targets and dependencies

$(call pdebug1)
$(call pdebug1,collect list of all targets)

loc_all_tgt :=

# add loc_single_, loc_comp_ and loc_dir_ targets
$(foreach type,$(all_types), \
	$(eval loc_all_tgt += $(loc_single_$(type)) $(loc_comp_$(type)) $(loc_dir_$(type))) \
)

# add dependencies for compound targets, adding .host.o for host-types
$(foreach type,$(all_types), \
	$(eval deps :=) \
	\
	$(foreach tgt,$(loc_comp_$(type)), \
		$(eval deps += $($(call hostbasename,$(tgt))-y)) \
	) \
	\
	$(if $(findstring host,$(type)),\
		$(eval loc_all_tgt += $(patsubst %.o,%.host.o,$(deps))) \
		, \
		$(eval loc_all_tgt += $(deps)) \
	) \
)

# remove duplicates
loc_all_tgt := $(sort $(loc_all_tgt))

$(call pdebug1,    loc_all_tgt: $(loc_all_tgt))


## update global list for $(all_types), e.g. bin

# add single and comp targets to the global lists (e.g. obj and lib), prefixing with $(loc_build_tree)
$(foreach type,$(all_types), \
	$(eval $(type) += $(addprefix $(loc_build_tree)/,$(loc_single_$(type)) $(loc_comp_$(type)))) \
)


####
## rule generation
####

$(call pdebug1)
$(call pdebug1,generate flag rules)


## flag rules

# target specific flag rules, i.e. <target>-*flags and <target>-*flags-y
# 	this applies to all files (direct targets and dependencies)
$(foreach tgt,$(loc_all_tgt), \
	$(foreach flag,$(all_flags), \
		$(call gen_rule_tgt_flags,$(tgt),$(flag),$(loc_build_tree)/) \
	) \
)

# local flags, i.e. *flags-y
$(foreach tgt,$(loc_all_tgt), \
	$(foreach flag,$(all_flags), \
		$(call gen_rule_loc_flags,$(tgt),$(flag),$(loc_build_tree)/) \
	) \
)

# local flags for ./obj.o and ./obj.host.o
$(foreach type,$(obj_types), \
	$(if $(loc_single_$(type))$(loc_comp_$(type))$(loc_dir_$($(type))), \
		$(foreach flag,$(all_flags), \
			$(call gen_rule_loc_flags,obj$(findstring .host,.$(type)).o,$(flag),$(loc_build_tree)/) \
		) \
	) \
)


## build rules

$(call pdebug1)
$(call pdebug1,generate compound target rules)

# compound host targets
$(call gen_rule_comp,    compile_o_o,          $(loc_comp_hostobj),              $(loc_build_tree)/,               host    )
$(call gen_rule_comp,    compile_o_o,          $(loc_comp_hostobj-nobuiltin),    $(loc_build_tree)/,               host    )
$(call gen_rule_comp,    compile_lib_o,        $(loc_comp_hostlib),              $(loc_build_tree)/,               host    )
$(call gen_rule_comp,    compile_bin_o,        $(loc_comp_hostbin),              $(loc_build_tree)/,               host    )

# normal host targets
$(call gen_rule_comp,    compile_o_o,          $(loc_comp_obj),                  $(loc_build_tree)/,                       )
$(call gen_rule_comp,    compile_o_o,          $(loc_comp_obj-nobuiltin),        $(loc_build_tree)/,                       )
$(call gen_rule_comp,    compile_lib_o,        $(loc_comp_lib),                  $(loc_build_tree)/,                       )
$(call gen_rule_comp,    compile_bin_o,        $(loc_comp_bin),                  $(loc_build_tree)/,                       )

$(call pdebug1)
$(call pdebug1,generate pattern rules)

# pattern rules

# C/C++/Asm -> .i
$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.host.S.i,      $(loc_src_tree)/%.S,            host    )
$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.host.i,        $(loc_src_tree)/%.S,            host    )
$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.host.i,        $(loc_src_tree)/%.c,            host    )
$(call gen_rule_basic,    compile_i_cxx,        $(loc_build_tree)/%.host.i,        $(loc_src_tree)/%.cc,           host    )

$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.S.i,           $(loc_src_tree)/%.S,                    )
$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.i,             $(loc_src_tree)/%.S,                    )
$(call gen_rule_basic,    compile_i_c,          $(loc_build_tree)/%.i,             $(loc_src_tree)/%.c,                    )
$(call gen_rule_basic,    compile_i_cxx,        $(loc_build_tree)/%.i,             $(loc_src_tree)/%.cc,                   )

# C/C++ -> .S
$(call gen_rule_basic,    compile_s_c,          $(loc_build_tree)/%.S,             $(loc_src_tree)/%.c,                    )
$(call gen_rule_basic,    compile_s_cxx,        $(loc_build_tree)/%.S,             $(loc_src_tree)/%.cc,                   )
$(call gen_rule_basic,    compile_s_c,          $(loc_build_tree)/%.host.S,        $(loc_src_tree)/%.c,            host    )
$(call gen_rule_basic,    compile_s_cxx,        $(loc_build_tree)/%.host.S,        $(loc_src_tree)/%.cc,           host    )

# C/C++ -> .o
$(call gen_rule_basic,    compile_o_s,          $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.host.S.i,   host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.host.o,        $(loc_src_tree)/%.c,            host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.host.o,        $(loc_src_tree)/%.cc,           host    )

$(call gen_rule_basic,    compile_o_s,          $(loc_build_tree)/%.o,             $(loc_build_tree)/%.S.i,                )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.o,             $(loc_src_tree)/%.c,                    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.o,             $(loc_src_tree)/%.cc,                   )

# yacc
ifeq ($(project_type),cxx)
$(call gen_rule_basic,    compile_c_y,          $(loc_build_tree)/%.tab.cc,        $(loc_src_tree)/%.y,                    )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.tab.h,         $(loc_build_tree)/%.tab.cc,             )
endif

$(call gen_rule_basic,    compile_c_y,          $(loc_build_tree)/%.tab.c,         $(loc_src_tree)/%.y,                    )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.tab.h,         $(loc_build_tree)/%.tab.c,              )

$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.tab.host.o,    $(loc_build_tree)/%.tab.cc,     host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.tab.host.o,    $(loc_build_tree)/%.tab.c,      host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.tab.cc,     host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.tab.c,      host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.tab.o,         $(loc_build_tree)/%.tab.cc,             )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.tab.o,         $(loc_build_tree)/%.tab.c,              )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.o,             $(loc_build_tree)/%.tab.cc,             )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.o,             $(loc_build_tree)/%.tab.c,              )

# lex
ifeq ($(project_type),cxx)
$(call gen_rule_basic,    compile_c_l,          $(loc_build_tree)/%.lex.cc,        $(loc_src_tree)/%.l,                    )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.lex.h,         $(loc_build_tree)/%.lex.cc,             )
endif

$(call gen_rule_basic,    compile_c_l,          $(loc_build_tree)/%.lex.c,         $(loc_src_tree)/%.l,                    )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.lex.h,         $(loc_build_tree)/%.lex.c,              )

$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.lex.host.o,    $(loc_build_tree)/%.lex.cc,     host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.lex.host.o,    $(loc_build_tree)/%.lex.c,      host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.lex.cc,     host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.lex.c,      host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.lex.o,         $(loc_build_tree)/%.lex.cc,             )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.lex.o,         $(loc_build_tree)/%.lex.c,              )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.o,             $(loc_build_tree)/%.lex.cc,             )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.o,             $(loc_build_tree)/%.lex.c,              )

# gperf
ifeq ($(project_type),cxx)
$(call gen_rule_basic,    compile_cxx_gperf,    $(loc_build_tree)/%.hash.cc,       $(loc_src_tree)/%.gperf,                )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.hash.h,        $(loc_build_tree)/%.hash.cc,            )
endif

$(call gen_rule_basic,    compile_c_gperf,      $(loc_build_tree)/%.hash.c,        $(loc_src_tree)/%.gperf,                )
$(call gen_rule_basic,    nop,                  $(loc_build_tree)/%.hash.h,        $(loc_build_tree)/%.hash.c,             )

$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.hash.host.o,   $(loc_build_tree)/%.hash.cc,    host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.hash.host.o,   $(loc_build_tree)/%.hash.c,     host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.hash.cc,    host    )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.host.o,        $(loc_build_tree)/%.hash.c,     host    )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.hash.o,        $(loc_build_tree)/%.hash.cc,            )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.hash.o,        $(loc_build_tree)/%.hash.c,             )
$(call gen_rule_basic,    compile_o_cxx,        $(loc_build_tree)/%.o,             $(loc_build_tree)/%.hash.cc,            )
$(call gen_rule_basic,    compile_o_c,          $(loc_build_tree)/%.o,             $(loc_build_tree)/%.hash.c,             )

$(call pdebug1)
$(call pdebug1,generate obj.o rules)

# obj.o
# 	in case no target objects are specified, generate a dummy obj.o
# 	the dummy is required since other wise a missing obj.o would be
# 	reported
$(if $(loc_single_obj)$(loc_comp_obj)$(loc_dir_obj), \
	$(call gen_rule_basic, compile_o_o, $(loc_build_tree)/obj.o, $(loc_dir_obj) $(addprefix $(loc_build_tree)/,$(loc_single_obj) $(loc_comp_obj))) \
	, \
	$(shell [ ! -e $(loc_build_tree)/obj.empty.c ] && { mkdir -p $(loc_build_tree); touch $(loc_build_tree)/obj.empty.c; }) \
	$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/obj.o, $(loc_build_tree)/obj.empty.c,) \
)

# obj.host.o
# 	same dummy object as above but for the host
$(if $(loc_single_hostobj)$(loc_comp_hostobj)$(loc_dir_hostobj), \
	$(call gen_rule_basic, compile_o_o, $(loc_build_tree)/obj.host.o, $(loc_dir_hostobj) $(addprefix $(loc_build_tree)/,$(loc_single_hostobj) $(loc_comp_hostobj)), host) \
	, \
	$(shell [ ! -e $(loc_build_tree)/obj.empty.c ] && { mkdir -p $(loc_build_tree); touch $(loc_build_tree)/obj.empty.c; }) \
	$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/obj.host.o, $(loc_build_tree)/obj.empty.c, host) \
)

# .o -> bin (corresponding pattern rules (<dir>/%: %.o) do not work, since '%:' matches everything)
# .o -> lib (corresponding pattern rules (<dir>/%.a: %.o) do not work since its not possible to differentiate between target and host libs)
$(foreach type,$(bin_types) $(lib_types), \
	$(foreach tgt,$(loc_single_$(type)), \
		$(call gen_rule_basic,    compile_$(findstring bin,$(type))$(findstring lib,$(type))_o,    $(loc_build_tree)/$(tgt),    $(loc_build_tree)/$(basename $(tgt))$(findstring .host,.$(type)).o,    $(findstring host,$(type))    ) \
	) \
)

$(call pdebug1)
$(call pdebug1,generate bin rules)


####
## cleanup
####

# clear target flag variables (<target>-<flag>, <target>-<flag>-y) to name collisions with targets with the same stem but in a different directory
$(foreach tgt,$(loc_all_tgt), \
	$(foreach flag,$(all_flags), \
		$(eval $(call hostbasename,$(tgt))-$(flag) :=) \
		$(eval $(call hostbasename,$(tgt))-$(flag)-y :=) \
	) \
)

# clear compound target dependency variables (<target>-y) to name collisions with targets with the same stem but in a different directory
$(foreach type,$(all_types), \
	$(foreach tgt,$(loc_comp_$(type)), \
		$(eval $(call hostbasename,$(tgt))-y :=) \
	) \
)

	
####
## subdirectory traversal
####

# handle subdir-*flags and subdir-*flags-y
# 	assign <subdir>-*flags and subdir-*flags-y to subdir-<subdir>-*flags to avoid overwriting them by name collisions
$(foreach dir,$(patsubst %/,%,$(subdir-y)), \
	$(foreach flag,$(all_flags), \
		$(eval subdir-$(loc_dir)/$(dir)-$(flag) := $($(dir)-$(flag)) $($(dir)-$(flag)-y) $(subdir-$(flag)) $(subdir-$(flag)-y)) \
		$(eval $(dir)-$(flag) :=) \
	) \
)

# include sub-directories to $(loc_src_tree) listed in $(subdir-y) and
# sub-directories to $(src_tree) listed in $(ext_dir)
$(call dinclude,$(addprefix $(loc_dir)/,$(subdir-y)) $(ext_dir))
