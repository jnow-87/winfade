bin-y := winfade

winfade-y := \
	main.o \
	opts.o \
	group.o \
	cmds.o \
	cmds.hash.o \
	common/

$(loc_build_tree)/main.o: $(loc_build_tree)/cmds.hash.h

winfade-ldlibs := -lX11 -lXrandr
cmds.hash-cflags-y := -Wno-missing-field-initializers
