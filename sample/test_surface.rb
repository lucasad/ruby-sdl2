require 'sdl2'

s = SDL2::Surface.load("icon.bmp")
p s.w
p s.h
p s.pitch
p s.bits_per_pixel
s2 = SDL2::Surface.from_string(s.pixels, 32, 32, 24)
