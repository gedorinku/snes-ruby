puts 'hello world'

KEY_UP = 2048
KEY_DOWN = 1024
KEY_RIGHT = 256
KEY_LEFT = 512

x = 0
y = 0
bg_y = 0

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

  # SNES::OAM.set(0, x, y, 3, 0, 0, 0, 0)

  bg_y += 2

  SNES::Bg.scroll(1, x, y)

  SNES.wait_for_vblank()
end
