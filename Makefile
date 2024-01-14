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

sfiles  = $(wildcard $(sources)/*.asm)
sfiles += $(wildcard $(sources)/*/*.asm)
sfiles += $(wildcard $(sources)/*/*/*.asm)

ofiles	:= $(cfiles:%.c=build/%.o) $(sfiles:%.asm=build/%.o)
listfiles:= $(cfiles:%.c=listing/%.asm) $(cfiles:%.c=listing/%.i)

LDFLAGS	:= -B -E -T -P00 -F hirom.cfg
libs	:= -LCL

includes:= $(foreach dir,$(incdirs),-I$(dir))
DEFINES := -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0

export VPATH	:= $(foreach dir,$(sources),$(CURDIR)/$(dir))
#--------------------------------------------------
.PHONY: all debug release $(listing) clean run run2

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

%.asm : %.png %.grit
	$(snesgrit) $<

# build/%.o : %.c
# 	$(CC) $(includes) $(CFLAGS) -O $@ $(subst /mnt/c,c:\,$<)

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
# $(LD) $(LDFLAGS) $(ofiles) -O $@ $(libs) -MN21
# $(LD) -HB -M21 -V -T -Pff \
#             -Zcode=C00000 -U0000,0000 \
#             -Aregistration_data=FFB0,7FB0 \
#             -Aressource=18000,8000 \
#             -N $(ofiles) $(libs) -O $@
	$(LD) -HB -M21 -V -T -Pff \
				-C8000 \
				-D7e2000,18000 \
				-KE00000,0000 \
                -Zcode=8000,ffff -U7f0000 \
                -Aregistration_data=FFB0,7FB0 \
                -Aressource=18000,8000 \
                -N $(ofiles) $(libs) -O $@
	@sed -e "s/^\(\w\)/;\1/g" $(target).map | sed -e "s/^\t//g" > $(target).sym
