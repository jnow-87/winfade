bin-y := windo

windo-y := \
	main.o \
	opts.o \
	cmds.o \
	cmds.hash.o \
	common/

$(loc_build_tree)/main.o: $(loc_build_tree)/cmds.hash.h

windo-ldlibs := -lX11 -lXrandr
cmds.hash-cflags-y := -Wno-missing-field-initializers
