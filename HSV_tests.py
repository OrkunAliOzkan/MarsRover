import numpy as np
from PIL import Image


img = Image.open('image.png')
pixels = np.array(img)

final = np.zeros(shape=(480, 640, 3))	#after all operations

def pre_assignments ():
	global red, green, blue
	red = int(pixels[y, x, 0])	#aliases for input
	green = int(pixels[y, x, 1])
	blue = int(pixels[y, x, 2])

def HSV_conversion ():
	global hue, sat, val, RGB_max
	RGB_max = red if (red > green and red > blue) else green if (green > blue) else blue
	RGB_min = red if (red < green and red < blue) else green if (green < blue) else blue
	RGB_diff = RGB_max - RGB_min if (RGB_max-RGB_min != 0) else 1
	val = RGB_max
	sat = 0 if val == 0 else (100 * RGB_diff) / RGB_max
	hue_diff = green-blue if (RGB_max == red) else 2+(blue-red) if (RGB_max == green) else 4+(red-green)
	hue = ((60 * hue_diff) / RGB_diff) % 360

def post_assignments ():
	final[y, x, 0] = RGB_max if RGB_max == red else 0
	final[y, x, 1] = RGB_max if RGB_max == green else 0
	final[y, x, 2] = RGB_max if RGB_max == blue else 0

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
out.save("out_HSV.png")