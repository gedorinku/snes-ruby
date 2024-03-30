export PVSNESLIB_HOME := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))pvsneslib
PVSNESLIB_DEBUG = 1

CFLAGS += -Isrc -Isrc/musl -DMRBC_USE_FLOAT=0 -DMRBC_ALLOC_LIBC=1

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

#---------------------------------------------------------------------------------
# ROMNAME is used in snes_rules file
export ROMNAME := hello_world

# all: src/main.rb.bytecode.c bitmaps $(ROMNAME).sfc
all: pvsneslib src/sa1/main.rb.bytecode.c bitmaps $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx
	# $(MAKE) -C $(PVSNESLIB_HOME) clean
	
#---------------------------------------------------------------------------------
pvsneslib: FORCE
	# FIXME:
	# $(MAKE) -C $(PVSNESLIB_HOME)

pvsneslibfont.pic: pvsneslibfont.png
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFXCONV) -n -gs8 -po16 -pc16 -pe0 -mR! -m! -fpng $<

bitmaps : pvsneslibfont.pic map_512_512.pic sprites.pic tiles.pic

src/sa1/%.ps: src/sa1/%.c
	@echo Compiling to .ps ... $(notdir $<)
	$(CC) $(CFLAGS) -sa1 -Wall -c $< -o $@
ifeq ($(DEBUG),1)
	cp $@ $@.01.dbg
endif

src/sa1/main.c : src/sa1/main.rb.bytecode.c
src/sa1/main.rb.bytecode.c : src/main.rb
	mrbc --remove-lv -Bmrbbuf -o $@ $<

FORCE:
