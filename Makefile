export PVSNESLIB_HOME := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))pvsneslib
PVSNESLIB_DEBUG = 1

CFLAGS += -Isrc/mrubyc -Isrc/musl -D MRBC_USE_FLOAT=0

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

#---------------------------------------------------------------------------------
# ROMNAME is used in snes_rules file
export ROMNAME := hello_world

# all: src/main.rb.bytecode.c bitmaps $(ROMNAME).sfc
all: pvsneslib src/main.rb.bytecode.c bitmaps $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx
	# $(MAKE) -C $(PVSNESLIB_HOME) clean
	
#---------------------------------------------------------------------------------
pvsneslib: FORCE
	# FIXME:
	# $(MAKE) -C $(PVSNESLIB_HOME)

pvsneslibfont.pic: pvsneslibfont.png
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFXCONV) -n -gs8 -po16 -pc16 -pe0 -mR! -m! -fpng $<

bitmaps : pvsneslibfont.pic

src/main.rb.bytecode.c: src/main.rb
	mrbc --remove-lv -Bmrbbuf -o src/main.rb.bytecode.c $<

FORCE:
