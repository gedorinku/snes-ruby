#include <snes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bg.h"

// extern char patterns, patterns_end;
// extern char palette;
// extern char map, map_end;
extern char tiles_patterns, tiles_patterns_end;
extern char tiles_palette;
extern uint16_t tiles_map, tiles_map_end;
extern char background_far_patterns, background_far_patterns_end;
extern char background_far_palette;
extern uint16_t background_far_map, background_far_map_end;
extern char gfxpsrite, gfxpsrite_end;
extern char palsprite, palsprite_end;

// extern char soundbrr, soundbrrend;
// brrsamples tadasound;

int main(void) {
  // spcBoot();

  consoleInit();

  bgInitTileSet(1, &tiles_patterns, &tiles_palette, 0,
                (&tiles_patterns_end - &tiles_patterns), 16 * 2, BG_16COLORS,
                0x4000);
  bgInitTileSet(2, &background_far_patterns, &background_far_palette, 1,
                (&background_far_patterns_end - &background_far_patterns),
                4 * 2, BG_16COLORS, 0x3000);

  bgInitMapSet(1, &tiles_map, (&tiles_map_end - &tiles_map), SC_64x64,
               SNES_BG2_TILE_MAP_VRAM_ADDR);
  // FIXME: 32bit - 32bit の引き算が正しくないのでハードコード
  bgInitMapSet(2, &background_far_map, 32 * 32 * 2, SC_32x32,
               SNES_BG3_TILE_MAP_VRAM_ADDR);

  // spcAllocateSoundRegion(39);

  oamInitGfxSet(&gfxpsrite, (&gfxpsrite_end - &gfxpsrite), &palsprite,
                (&palsprite_end - &palsprite), 0, 0x0000, OBJ_SIZE32_L64);
  oamSet(0, 0, 0, 3, 0, 0, 0, 0);
  oamSetEx(0, OBJ_SMALL, OBJ_SHOW);
  oamSetVisible(0, OBJ_SHOW);

  setMode(BG_MODE1, 0);
  bgSetDisable(0);
  setScreenOn();

  // spcSetSoundEntry(15, 15, 4, &soundbrrend - &soundbrr, &soundbrr, &tadasound);

  while (1) {
    listen_call_from_sa1();
  }
  return 0;
}
