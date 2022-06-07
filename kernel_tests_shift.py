import numpy as np
from PIL import Image

img = Image.open('image.png')
pixels = np.array(img)

r_buff = [0 for i in range(7)]		#7 pixels
g_buff = [0 for i in range(7)]
b_buff = [0 for i in range(7)]

horiz_r = np.zeros(shape=(7, 640))	#7 rows
horiz_g = np.zeros(shape=(7, 640))	#7 rows
horiz_b = np.zeros(shape=(7, 640))	#7 rows

final = np.zeros(shape=(480, 640, 3))	#after all operations

r_blur_x = g_blur_x = b_blur_x = 0
r_blur_y = g_blur_y = b_blur_y = 0

r_diff_x = g_diff_x = b_diff_x = 0
r_sum_x = g_sum_x = b_sum_x = 0


def pre_assignments ():
	global red, green, blue
	red = pixels[y, x, 0]	#aliases for input
	green = pixels[y, x, 1]
	blue = pixels[y, x, 2]

def delay_line_input ():
	for i in range(6, 0, -1):
		r_buff[i] = r_buff[i-1]	#move items in delay line
		g_buff[i] = g_buff[i-1]
		b_buff[i] = b_buff[i-1]
	r_buff[0] = red		#new values come in
	g_buff[0] = green
	b_buff[0] = blue

def delay_line_horiz ():
	for i in range(6, 0, -1):
		horiz_r[i] = horiz_r[i-1]	#move rows in delay line
		horiz_g[i] = horiz_g[i-1]
		horiz_b[i] = horiz_b[i-1]
	horiz_r[0] = np.zeros(shape=(640))	#for new row
	horiz_g[0] = np.zeros(shape=(640))
	horiz_b[0] = np.zeros(shape=(640))


def accumulate ():
	global r_blur_x, g_blur_x, b_blur_x, r_diff_x, g_diff_x, b_diff_x, r_sum_x, g_sum_x, b_sum_x
	r_diff_x = red - int(r_buff[6]) if x > 6 else red
	g_diff_x = green - int(g_buff[6]) if x > 6 else green
	b_diff_x = blue - int(b_buff[6]) if x > 6 else blue

	if x == 0:
		r_sum_x = 0
		g_sum_x = 0
		b_sum_x = 0
	r_sum_x = r_sum_x + r_diff_x
	g_sum_x = g_sum_x + g_diff_x
	b_sum_x = b_sum_x + b_diff_x
	r_blur_x = r_sum_x / 7		#divide
	g_blur_x = g_sum_x / 7
	b_blur_x = b_sum_x / 7

def avg_horiz ():
	global r_blur_x, g_blur_x, b_blur_x
	r_blur_x = sum(r_buff) / 7			#avg pixels from left
	g_blur_x = sum(g_buff) / 7
	b_blur_x = sum(b_buff) / 7

def avg_vert ():
	global r_blur_y, g_blur_y, b_blur_y
	r_blur_y = 0
	g_blur_y = 0
	b_blur_y = 0
	for i in range(7):	#avg pixels from above cols
		r_blur_y = r_blur_y + horiz_r[i, x]
		g_blur_y = g_blur_y + horiz_g[i, x]
		b_blur_y = b_blur_y + horiz_b[i, x]
	r_blur_y = r_blur_y / 7
	g_blur_y = g_blur_y / 7
	b_blur_y = b_blur_y / 7

def post_assignments ():
	horiz_r[0, x] = r_blur_x	#writes to newest row of horiz
	horiz_g[0, x] = g_blur_x
	horiz_b[0, x] = b_blur_x
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
			delay_line_horiz()
		else:
			x += 1
		if y != 480:
			pre_assignments()
			#accumulate()
			#if (i < 1): print(red, r_buff[6], r_diff_x)########
			avg_horiz()
			avg_vert()
			delay_line_input()
			post_assignments()

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("out.png")