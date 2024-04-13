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
  # FIXME: const が参照できない?
  WIDTH = 24
  # GAP = 10 * 8

  attr_reader :x, :y

  def initialize(x, y)
    @x = x
    @y = y
    @gap = 10 * 8
  end

  def render(tile_maps, offset_tiles)
    gap_tile_x = x / 8
    gap_tile_y = y / 8

    block_tile_x = gap_tile_x
    upper_block_tile_y = gap_tile_y - 1

    i = 0
    while i < BLOCK_MAP.size && 0 <= upper_block_tile_y - i
      j = 0
      while j < BLOCK_MAP[i].size
        tile_maps[32 * (upper_block_tile_y - i) + j + block_tile_x - offset_tiles] = BLOCK_MAP[i][j]
        j += 1
      end
      i += 1
    end

    lower_block_tile_y = gap_tile_y + @gap / 8

    i = 0
    while i < BLOCK_MAP.size && lower_block_tile_y + i < 32
      j = 0
      while j < BLOCK_MAP[i].size
        tile_maps[32 * (lower_block_tile_y + i) + j + block_tile_x - offset_tiles] = BLOCK_MAP[i][j]
        j += 1
      end
      i += 1
    end
  end

  def intersects?(player)
    @x <= player.x + player.width && player.x <= @x + WIDTH &&
      (player.y <= @y || @y + @gap <= player.y + player.height)
  end

  def behind_of?(player)
    @x + WIDTH < player.x
  end
end

# @param [Array<BlockPair>] block_pairs
# @param [Integer] offset
# @param [Array<Integer>] tile_maps
def render_block_pairs(block_pairs, offset, tile_maps)
  i = 0
  while i < block_pairs.size
    block_pairs[i].render(tile_maps, offset)
    i += 1
  end
end

# @param [Range] x_range
# @return [Array<BlockPair>]
def generate_block_pairs(x_range)
  x = x_range.first
  step = 8 * 8

  res = []

  x = step
  i = 0
  while i < 2
    tmp = []
    x_end = 32 * 8 * (i + 1)
    while x < x_end
      tile_y = 1 + SNES.rand(15)
      tmp << BlockPair.new(x, tile_y * 8)

      x += step
    end

    tile_maps = Array.new(32 * 32, 0)
    offset = tile_maps.size * i
    render_block_pairs(tmp, offset, tile_maps)
    SNES::Bg.update_tile_map(1, offset, tile_maps)

    res += tmp

    i += 1
  end

  res
end

block_pairs = generate_block_pairs(0...(32*8))

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

current_block_idx = 0
current_block = block_pairs.first

game_state = :running

while true
  SNES::Pad.wait_for_scan
  pad = SNES::Pad.current(0)

  if game_state == :running
    if pad & KEY_A != 0
      player_dy = -60
    end

    camera_x += 2

    player_dy += 7
    player_dy = 200 if 200 < player_dy
    player.y += player_dy / 10
    player.x = camera_x
    player.render(camera_x, camera_y)

    if current_block.intersects?(player) || player.y < 0 || MAX_Y <= player.y
      game_state = :game_over
      SNES::SPC.play_sound(0)
    end

    if current_block.behind_of?(player)
      current_block_idx += 1
      current_block_idx %= block_pairs.size
      current_block = block_pairs[current_block_idx]
    end
  else
    if pad & KEY_A != 0
      player = Player.new(0, MAX_Y / 2, 8, 8)
      camera_x = 0
      camera_y = 0
      player_dy = 0
      game_state = :running
    end
  end

  SNES::Bg.scroll(1, camera_x, camera_y)

  SNES::SPC.process
  SNES.wait_for_vblank
end
