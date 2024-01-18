ifeq ($(strip $(WDC_INCLUDE_DIR)),)
$(error "WDC_INCLUDE_DIR environment variable is not defined correctly")
endif

ifeq ($(strip $(WDC_LIB)),)
$(error "WDC_LIB environment variable is not defined correctly")
endif

CC	:= WDC816CC.exe
AS	:= WDC816AS.exe
LD	:= WDCLN.exe
PP 	:= gcc
C99CONV := c99conv.exe

target	:= $(shell basename $(CURDIR))
output	:= $(target).sfc
build	:= build
listing	:= listing
sources	:= src
incdirs	:= $(sources) $(build) src/mrubyc

cfiles = $(wildcard $(sources)/*.c)
cfiles += $(wildcard $(sources)/*/*.c)
cfiles += $(wildcard $(sources)/*/*/*.c)
cfiles += $(sources)/mrb_bytecode.c

rubyfiles = $(wildcard $(sources)/*.rb)
rubyfiles += $(wildcard $(sources)/*/*.rb)
rubyfiles += $(wildcard $(sources)/*/*/*.rb)

sfiles  = $(wildcard $(sources)/*.asm)
sfiles += $(wildcard $(sources)/*/*.asm)
sfiles += $(wildcard $(sources)/*/*/*.asm)

ofiles	:= $(cfiles:%.c=build/%.o) $(sfiles:%.asm=build/%.o)
listfiles:= $(cfiles:%.c=listing/%.asm) $(cfiles:%.c=listing/%.i)

LDFLAGS	:= -HB -V -T -Pff -Zcode=8000 -C8000 -K8000 -D7e2000,0000 -U7e8000,0000
libs	:= -LMS -LCL

includes:= $(foreach dir,$(incdirs),-I$(dir))
DEFINES := -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0 -DMRBC_ALLOC_LIBC=1

export VPATH	:= $(foreach dir,$(sources),$(CURDIR)/$(dir))
#--------------------------------------------------
.PHONY: all debug release $(listing) clean run run2
.PRECIOUS: $(listfiles) $(cfiles:%.c=listing/%.conv.i)

all: release

debug: CFLAGS	:= -ML -SI -SP -WL -WP -DDEBUG $(DEFINES)
debug: C2SFLAGS	:= -ML -AT -SI -SP -WL -WP -DDEBUG $(DEFINES)
debug: ASFLAGS	:= -S -DDEBUG
debug: $(listing) $(listfiles) $(output)

release: CFLAGS		:= -ML -SOP -WL -WP $(DEFINES)
release: C2SFLAGS	:= -ML -AT -SOP -WL -WP $(DEFINES)
release: ASFLAGS	:= -S
release: $(listing) $(listfiles) $(output)

$(build):
	@[ -d $@ ] || mkdir -p $@

$(output): $(build) $(ofiles)


$(listing): $(cfiles)
	@[ -d $@ ] || mkdir -p $@

clean:
	rm -rf $(build) $(output) $(target).bnk $(target).sym $(target).map $(listing)

src/mrb_bytecode.c : $(rubyfiles)
	mrbc -s --remove-lv -Bmrbbuf -o $@ $<
	sed -i '1i#include "int8.h"' $@

build/%.o : listing/%.asm
	mkdir -p $(dir $@)
	$(AS) $(includes) $(ASFLAGS) -O $@ $(subst /mnt/c,c:\,$<)

build/%.o : %.asm
	mkdir -p $(dir $@)
	$(AS) $(includes) $(ASFLAGS) -O $@ $(subst /mnt/c,c:\,$<)

listing/%.asm : listing/%.conv.i
	$(CC) $(includes) $(C2SFLAGS) -O $@ $(subst /mnt/c,c:\,$<)


listing/%.conv.i : listing/%.i
	$(C99CONV) $< $@ 2>&1 | tee $@.log
	! cat $@.log | grep -q -v ": error: "

listing/%.i : %.c
	mkdir -p $(dir $@)
	$(PP) -I$(WDC_INCLUDE_DIR) $(includes) $(DEFINES) -E -o $@ $(subst /mnt/c,c:\,$<)
	sed -i -e 's/^# \([0-9]* \)/\/\/ \1/g' $@

%.sfc:
	$(LD) $(LDFLAGS) \
				-N $(ofiles) $(libs) -O $@
	@sed -e "s/^\(\w\)/;\1/g" $(target).map | sed -e "s/^\t//g" | sed "s/~//g" > $(target).sym
