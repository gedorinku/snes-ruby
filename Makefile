CC	:= WDC816CC.exe
AS	:= WDC816AS.exe
LD	:= WDCLN.exe

target	:= $(shell basename $(CURDIR))
output	:= $(target).sfc
build	:= build
listing	:= listing
# sources	:= src
sources	:= src src/mrubyc
incdirs	:= $(sources) $(build)

cfiles	:= $(foreach dir,$(sources),$(notdir $(wildcard $(dir)/*.c)))
sfiles	:= $(foreach dir,$(sources),$(notdir $(wildcard $(dir)/*.asm)))
ofiles	:= $(cfiles:%.c=build/%.o) $(sfiles:%.asm=build/%.o)
listfiles:= $(cfiles:%.c=listing/%.asm)

LDFLAGS	:= -B -E -T -P00 -F hirom.cfg
libs	:= -LCL

includes:= $(foreach dir,$(incdirs),-I$(dir))

export VPATH	:= $(foreach dir,$(sources),$(CURDIR)/$(dir))
#--------------------------------------------------
.PHONY: all debug release $(listing) clean run run2

all: release

debug: CFLAGS	:= -ML -SI -SP -WL -WP -DDEBUG -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0
debug: C2SFLAGS	:= -ML -AT -SI -SP -WL -WP -DDEBUG -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0
debug: ASFLAGS	:= -S -DDEBUG
debug: $(listing) $(listfiles) $(output)

release: CFLAGS		:= -ML -SOP -WL -WP -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0
release: C2SFLAGS	:= -ML -AT -SOP -WL -WP -DSIZEOF_INT=2 -DSIZEOF_LONG=4 -DMRBC_USE_FLOAT=0
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

build/%.o : %.asm
	$(AS) $(includes) $(ASFLAGS) -O $@ $(subst /mnt/c,c:\,$<)

listing/%.asm : %.c
	$(CC) $(includes) $(C2SFLAGS) -O $@ $(subst /mnt/c,c:\,$<)

%.sfc:
	$(LD) $(LDFLAGS) $(ofiles) -O $@ $(libs)
	@sed -e "s/^\(\w\)/;\1/g" $(target).map | sed -e "s/^\t//g" > $(target).sym
