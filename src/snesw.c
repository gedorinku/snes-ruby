#include <snes.h>

void snesw_pads_current(u16 *dst, u16 value) {
  *dst = padsCurrent(value);
}
