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
  GAP = 10 * 8
  # FIXME: 2個目以降の const が参照できない?
  # WIDTH = 24

  attr_reader :x, :y

  def initialize(x, y)
    @x = x
    @y = y
    @width = 24
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

    lower_block_tile_y = gap_tile_y + GAP / 8

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

  def intersects?(player)
    @x <= player.x + player.width && player.x <= @x + @width &&
      (player.y <= @y || @y + GAP <= player.y + player.height)
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

class Player
  attr_accessor :x, :y, :width, :height

  def initialize(x, y, width, height)
    @x = x
    @y = y
    @width = width
    @height = height
  end

  def render(camera_x, camera_y)
    SNES::OAM.set(0, @x - camera_x, @y - camera_y, 3, 0, 0, 0, 0)
  end
end

player = Player.new(0, MAX_Y / 2, 8, 8)

camera_x = 0
camera_y = 0

# TODO: Playerにいれる
player_dy = 0

while true
  SNES::Pad.wait_for_scan
  pad = SNES::Pad.current(0)

  if pad & KEY_UP != 0
    camera_y -= 2
  end
  if pad & KEY_DOWN != 0
    camera_y += 2
  end
  if pad & KEY_RIGHT != 0
    camera_x += 2
  end
  if pad & KEY_LEFT != 0
    camera_x -= 2
  end

  if pad & KEY_A != 0
    player_dy = -75
  end

  player_dy += 3
  player_dy = 200 if 200 < player_dy
  # player.y += player_dy / 10
  player.y = camera_y + MAX_Y / 2
  player.x = camera_x
  player.render(camera_x, camera_y)

  if block_pairs.first.intersects?(player)
    SNES::SPC.play_sound(0)
  end

  SNES::Bg.scroll(1, camera_x, camera_y)

  SNES::SPC.process
  SNES.wait_for_vblank
end
