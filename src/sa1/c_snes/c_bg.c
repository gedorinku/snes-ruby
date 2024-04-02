#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"
#include "snesw.h"

static void c_snes_bg_scroll(mrbc_vm *vm, mrbc_value v[], int argc) {
  // if (argc != 3) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }
  // if (mrbc_type(v[1]) != MRBC_TT_INTEGER ||
  //     mrbc_type(v[2]) != MRBC_TT_INTEGER ||
  //     mrbc_type(v[3]) != MRBC_TT_INTEGER) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }

  call_s_cpu(bgSetScroll, sizeof(u8) + sizeof(u16) * 2, (u8)v[1].i, (u16)v[2].i,
             (u16)v[3].i);
}

static const u16 *default_tile_maps[4];
static size_t default_tile_map_sizes[4];
static u16 *tile_map_vram_addrs[4];

static void c_snes_bg_get_default_tile_map(mrbc_vm *vm, mrbc_value v[],
                                           int argc) {
  const int bg = v[1].i;
  const size_t n = default_tile_map_sizes[bg] / 2;
  mrbc_value res = mrbc_array_new(vm, n);

  int i;
  for (i = 0; i < n; i++) {
    mrbc_array_set(&res, i, &mrbc_integer_value(default_tile_maps[bg][i]));
  }

  SET_RETURN(res);
}

static void c_snes_bg_update_tile_map(mrbc_vm *vm, mrbc_value v[], int argc) {
  const int bg = v[1].i;
  const size_t n = v[2].array->n_stored;

  static u16 *buf;
  static size_t buf_n;

  if (buf_n < n) {
    if (buf != NULL) {
      sa1_free(buf);
    }

    buf = sa1_malloc(sizeof(u16) * n);
    buf_n = n;
  }

  int i;
  for (i = 0; i < n; i++) {
    buf[i] = v[2].array->data[i].i;
  }

  const u16 addr = tile_map_vram_addrs[bg];
  call_s_cpu(snesw_wait_and_dma_to_vram, sizeof(char *) + sizeof(u16) * 2, buf,
             addr, (u16)(n * 2));
}

void snes_init_class_bg(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "Bg", NULL);
  mrbc_define_method(vm, cls, "scroll", c_snes_bg_scroll);
  mrbc_define_method(vm, cls, "default_tile_maps",
                     c_snes_bg_get_default_tile_map);
  mrbc_define_method(vm, cls, "update_tile_map", c_snes_bg_update_tile_map);
}

void snes_bg_set_default_tile_map(int bg, const u8 *map, size_t size,
                                  u16 vram_addr) {
  default_tile_maps[bg] = map;
  default_tile_map_sizes[bg] = size;
  tile_map_vram_addrs[bg] = vram_addr;
}
