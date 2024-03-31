KEY_A = 128
KEY_UP = 2048
KEY_DOWN = 1024
KEY_RIGHT = 256
KEY_LEFT = 512

MAX_Y = 224 - 16

tile_maps = SNES::Bg.default_tile_maps(1)
tile_maps[2] = 3

x = 0
y = 0
player_dy = 0
player_y = MAX_Y / 2

while true
  SNES::Pad.wait_for_scan
  pad = SNES::Pad.current(0)

  if pad & KEY_UP != 0
    y -= 2
  end
  if pad & KEY_DOWN != 0
    y += 2
  end
  if pad & KEY_RIGHT != 0
    x += 2
  end
  if pad & KEY_LEFT != 0
    x -= 2
  end

  if pad & KEY_A != 0
    player_dy = -75
  end

  player_dy += 3
  player_dy = 200 if 200 < player_dy
  player_y += player_dy / 10
  # puts "y: #{player_y}, dy: #{player_dy}"
  SNES::OAM.set(0, 0, player_y, 3, 0, 0, 0, 0)

  SNES::Bg.scroll(1, x, y)

  SNES::Bg.update_tile_map(1, tile_maps)

  SNES::SPC.process
  SNES.wait_for_vblank
end
