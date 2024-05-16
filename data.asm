.include "hdr.asm"

.section ".rodata1" superfree

patterns:
.incbin "map_512_512.pic"
patterns_end:

.ends

.section ".rodata2" superfree
map:
.incbin "map_512_512.map"
map_end:

palette:
.incbin "map_512_512.pal"

gfxpsrite:
.incbin "sprites.pic"
gfxpsrite_end:

palsprite:
.incbin "sprites.pal"
palsprite_end:

; soundbrr:
; .incbin "tada.brr"
; soundbrrend:

.ends

.section ".rodata3" superfree

snesfont:
.incbin "pvsneslibfont.pic"

snesfontpal:
.incbin "pvsneslibfont.pal"

.ends

.section ".rodata4" superfree

tiles_patterns:
.incbin "tiles.pic"
tiles_patterns_end:

tiles_map:
.incbin "tiles.map"
tiles_map_end:

tiles_palette:
.incbin "tiles.pal"

.ends

.section ".rodata5" superfree

background_far_patterns:
.incbin "background_far.pic"
background_far_patterns_end:

background_far_map:
.incbin "background_far.map"
background_far_map_end:

background_far_palette:
.incbin "background_far.pal"

.ends
