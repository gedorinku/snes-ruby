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
  # GAP = 10 * 8
  # WIDTH = 24

  attr_reader :x, :y

  def initialize(x, y)
    @x = x
    @y = y
    @gap = 10 * 8
    @width = 24

    p "new, #{@gap}"
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

    p "@gap: #{@gap}, #{@gap.object_id % 0x10000}"
    lower_block_tile_y = gap_tile_y + @gap / 8

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
      (player.y <= @y || @y + @gap <= player.y + player.height)
  end
end

# @param [Array<BlockPair>] block_pairs
# @param [Array<Integer>] tile_maps
def render_block_pairs(block_pairs, tile_maps)
  i = 0
  while i < block_pairs.size
    puts "block_pairs[#{i}]: #{block_pairs[i].object_id.to_s(16)}"
    block_pairs[i].render(tile_maps)
    i += 1
  end
end

# @param [Range] x_range
# @return [Array<BlockPair>]
def generate_block_pairs(x_range)
  x = x_range.first
  step = 8 * 8

  res = []

  begin
    x += step
    tile_y = 1 + SNES.rand(30)
    # tile_y = 1
    res << BlockPair.new(x, tile_y * 8)
  end while x < x_range.last

  res
end

tile_maps = Array.new(32 * 32, 0)
block_pairs = generate_block_pairs(0...(32*8))
p "block_pairs.size: #{block_pairs.size}"
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

  # if pad & KEY_UP != 0
  #   camera_y -= 2
  # end
  # if pad & KEY_DOWN != 0
  #   camera_y += 2
  # end
  # if pad & KEY_RIGHT != 0
  #   camera_x += 2
  # end
  # if pad & KEY_LEFT != 0
  #   camera_x -= 2
  # end

  camera_x += 2

  if pad & KEY_A != 0
    player_dy = -75
  end

  player_dy += 3
  player_dy = 200 if 200 < player_dy
  player.y += player_dy / 10
  # player.y = camera_y + MAX_Y / 2
  player.x = camera_x
  player.render(camera_x, camera_y)

  if block_pairs.first.intersects?(player)
    SNES::SPC.play_sound(0)
  end

  SNES::Bg.scroll(1, camera_x, camera_y)

  SNES::SPC.process
  SNES.wait_for_vblank
end
