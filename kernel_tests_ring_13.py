import numpy as np
from PIL import Image

img = Image.open('image.png')
pixels = np.array(img)

gauss = [1, 6, 28, 89, 205, 338, 400, 338, 205, 89, 28, 6, 1]

r_ring = [0 for i in range(13)]		#13 pixels
g_ring = [0 for i in range(13)]
b_ring = [0 for i in range(13)]
count_input = 0

horiz_ring_r = np.zeros(shape=(7681))	#12 rows + 1 pixel
horiz_ring_g = np.zeros(shape=(7681))
horiz_ring_b = np.zeros(shape=(7681))
count_horiz = 0

final = np.zeros(shape=(480, 640, 3))	#after all operations

r_blur_x = g_blur_x = b_blur_x = 0
r_blur_y = g_blur_y = b_blur_y = 0
r_sum_x = g_sum_x = b_sum_x = 0
r_sum_y = g_sum_y = b_sum_y = 0


def pre_assignments ():
	global red, green, blue
	red = pixels[y, x, 0]	#aliases for input
	green = pixels[y, x, 1]
	blue = pixels[y, x, 2]

def input_ring ():
	global count_input
	if count_input == 12: count_input = 0 	# % 13
	else: count_input += 1
	r_ring[count_input] = red		#new values come in
	g_ring[count_input] = green
	b_ring[count_input] = blue

def horiz_ring ():
	global count_horiz
	if count_horiz == 7680: count_horiz = 0 	# % 7681
	else: count_horiz += 1

def avg_horiz ():
	global  r_sum_x, g_sum_x, b_sum_x, r_blur_x, g_blur_x, b_blur_x
	r_sum_x = 0
	g_sum_x = 0
	b_sum_x = 0
	for i in range(13):
		r_sum_x += gauss[i]*r_ring[(count_input+i)%13]
		g_sum_x += gauss[i]*g_ring[(count_input+i)%13]
		b_sum_x += gauss[i]*b_ring[(count_input+i)%13]
	r_blur_x = r_sum_x / 1734
	g_blur_x = g_sum_x / 1734
	b_blur_x = b_sum_x / 1734

def avg_vert ():
	global r_sum_y, g_sum_y, b_sum_y, r_blur_y, g_blur_y, b_blur_y
	r_sum_y = 0
	g_sum_y = 0
	b_sum_y = 0
	for i in range(13):
		r_sum_y += gauss[i]*horiz_ring_r[(count_horiz+(640*i))%7681]
		g_sum_y += gauss[i]*horiz_ring_g[(count_horiz+(640*i))%7681]
		b_sum_y += gauss[i]*horiz_ring_b[(count_horiz+(640*i))%7681]
	r_blur_y = r_sum_y / 1734
	g_blur_y = g_sum_y / 1734
	b_blur_y = b_sum_y / 1734

def post_assignments ():
	horiz_ring_r[count_horiz] = r_blur_x	#writes to newest row of horiz
	horiz_ring_g[count_horiz] = g_blur_x
	horiz_ring_b[count_horiz] = b_blur_x
	final[y, x, 0] = r_blur_y	#writes to current row of final
	final[y, x, 1] = g_blur_y
	final[y, x, 2] = b_blur_y

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
			#accumulate()
			#if (i < 1): print(red, r_ring[6], r_diff_x)########
			avg_horiz()
			avg_vert()
			input_ring()
			horiz_ring()
			post_assignments()

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("out.png")