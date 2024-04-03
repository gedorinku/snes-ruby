KEY_A = 128
KEY_UP = 2048
KEY_DOWN = 1024
KEY_RIGHT = 256
KEY_LEFT = 512

MAX_Y = 224 - 16

BLOCK_MAP = [
  [160, 161, 165],
  [32, 33, 37],
  [32, 33, 37],
]

class BlockPair
  GAP = 10

  attr_reader :x, :y

  def initialize(x, y)
    @x = x
    @y = y
  end

  def render(tile_maps)
    gap_tile_x = x / 8
    gap_tile_y = y / 8

    block_tile_x = gap_tile_x
    upper_block_tile_y = gap_tile_y - 1

    i = 0
    while i < BLOCK_MAP.size && 0 <= upper_block_tile_y - i
      j = 0
      while j < BLOCK_MAP[i].size
        tile_maps[32 * (upper_block_tile_y - i) + j + block_tile_x] = BLOCK_MAP[i][j]
        j += 1
      end
      i += 1
    end

    lower_block_tile_y = gap_tile_y + GAP

    i = 0
    while i < BLOCK_MAP.size && lower_block_tile_y + i < 32
      j = 0
      while j < BLOCK_MAP[i].size
        tile_maps[32 * (lower_block_tile_y + i) + j + block_tile_x] = BLOCK_MAP[i][j]
        j += 1
      end
      i += 1
    end
  end
end

block_pairs = [
  BlockPair.new(48, 24),
  BlockPair.new(96, 40),
]

tile_maps = Array.new(32 * 32, 0)

def render_block_pairs(block_pairs, tile_maps)
  i = 0
  while i < block_pairs.size
    block_pairs[i].render(tile_maps)
    i += 1
  end
end

render_block_pairs(block_pairs, tile_maps)

SNES::Bg.update_tile_map(1, tile_maps)

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

  SNES::SPC.process
  SNES.wait_for_vblank
end
