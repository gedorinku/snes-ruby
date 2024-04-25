KEY_A = 128
KEY_UP = 2048
KEY_DOWN = 1024
KEY_RIGHT = 256
KEY_LEFT = 512

MAX_Y = 224 - 16
START_X = 32

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
    while 0 <= upper_block_tile_y - i
      map_y = i < BLOCK_MAP.size ? i : BLOCK_MAP.size - 1

      j = 0
      while j < BLOCK_MAP[map_y].size
        tile_maps[32 * (upper_block_tile_y - i) + j + block_tile_x - offset_tiles] = BLOCK_MAP[map_y][j]
        j += 1
      end
      i += 1
    end

    lower_block_tile_y = gap_tile_y + @gap / 8

    vflip_flag = 0x8000

    i = 0
    while lower_block_tile_y + i < 32
      map_y = i < BLOCK_MAP.size ? i : BLOCK_MAP.size - 1

      j = 0
      while j < BLOCK_MAP[map_y].size
        tile_maps[32 * (lower_block_tile_y + i) + j + block_tile_x - offset_tiles] = BLOCK_MAP[map_y][j] + vflip_flag
        j += 1
      end
      i += 1
    end
  end

  def intersects?(player)
    box_x = player.box_x
    box_y = player.box_y
    @x <= box_x + player.width && box_x <= @x + WIDTH &&
      (box_y <= @y || @y + @gap <= box_y + player.height)
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

# @return [Array<BlockPair>]
def generate_block_pairs
  step = 8 * 8

  res = []

  x = step + START_X
  i = 0
  while i < 2
    tmp = []
    x_end = 32 * 8 * (i + 1)
    while x < x_end
      tile_y = 1 + SNES.rand(15)
      tmp << BlockPair.new(x, tile_y * 8)

      x += step
    end

    tile_maps = Array.new(32 * 32, 18)
    offset = tile_maps.size * i
    render_block_pairs(tmp, offset, tile_maps)
    SNES::Bg.update_tile_map(1, offset, tile_maps)

    res += tmp

    i += 1
  end

  res
end

block_pairs = generate_block_pairs

class Player
  attr_accessor :x, :y, :width, :height

  def initialize(x, y)
    @x = x
    @y = y
    @width = 16
    @height = 14
  end

  def box_x
    @x + 2
  end

  def box_y
    @y + 5
  end

  def render(camera_x, camera_y, frame)
    SNES::OAM.set(0, @x - camera_x, @y - camera_y, 3, 0, 0, frame, 0)
  end
end

player = Player.new(START_X, MAX_Y / 2)

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
    player.x = camera_x + START_X
    player.render(camera_x, camera_y, player_dy < 0 ? 0 : 4)

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
      player = Player.new(START_X, MAX_Y / 2)
      camera_x = 0
      camera_y = 0
      player_dy = 0
      game_state = :running
    end
  end

  SNES::Bg.scroll(1, camera_x, camera_y)
  if camera_x & 1 == 0
    SNES::Bg.scroll(2, camera_x >> 1, 0)
  end

  SNES::SPC.process
  SNES.wait_for_vblank
end
