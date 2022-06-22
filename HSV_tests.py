import numpy as np
from PIL import Image


img = Image.open('inputs/wheel2.png')
pixels = np.array(img)

final = np.zeros(shape=(480, 640, 3))	#after all operations

def pre_assignments ():
	global red, green, blue, grey
	red = int(pixels[y, x, 0])	#aliases for input
	green = int(pixels[y, x, 1])
	blue = int(pixels[y, x, 2])
	grey = red/4 + green/2 + blue/4

def HSV_conversion ():
	global Hue, Sat, Val, RGB_max
	RGB_max = blue if (blue > green and blue > red) else green if (green > red) else red
	RGB_min = blue if (blue < green and blue < red) else green if (green < red) else red
	RGB_diff = RGB_max - RGB_min if (RGB_max-RGB_min != 0) else 1
	Val = RGB_max
	Sat = 0 if Val == 0 else (100 * RGB_diff) / RGB_max
	Hue_0 = (60*(green-blue)/RGB_diff) if (RGB_max == red) else (120+(60*(blue-red)/RGB_diff)) if (RGB_max == green) else (240+(60*(red-green)/RGB_diff))
	Hue = Hue_0 + 360 if (Hue_0<0) else Hue_0 - 360 if (Hue_0>360) else Hue_0

def post_assignments ():
	final[y, x, 0] = red if (Hue<10 or Hue >350) else grey
	final[y, x, 1] = green if (Hue<10 or Hue >350) else grey
	final[y, x, 2] = blue if (Hue<10 or Hue >350) else grey

x = 0
y = 0

for i in range(480):
	for j in range(640):
		if j == 639:
			x = 0
			y += 1
		else:
			x += 1
		if y != 480:
			pre_assignments()
			HSV_conversion()
			post_assignments()

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("results/out_HSV.png")