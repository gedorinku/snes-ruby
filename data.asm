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

soundbrr:
.incbin "tada.brr"
soundbrrend:

.ends

.section ".rodata3" superfree

snesfont:
.incbin "pvsneslibfont.pic"

snesfontpal:
.incbin "pvsneslibfont.pal"

.ends
