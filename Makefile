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

background_far.pic: background_far.bmp
	@echo convert bitmap ... $(notdir $<)
	$(GFX4CONV) -s 8 -o 4 -u 4 -e 4 -p -m -t bmp -i $<

%.pic: %.bmp
	@echo convert bitmap ... $(notdir $<)
	$(GFX4CONV) -s 8 -o 16 -u 16 -R -e 0 -p -m -t bmp -i $<

bitmaps : pvsneslibfont.pic map_512_512.pic sprites.pic tiles.pic background_far.pic

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
