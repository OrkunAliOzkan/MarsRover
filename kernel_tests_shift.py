import time
import numpy as np
from PIL import Image

start = time.time()

img = Image.open('image.png')
pixels = np.array(img)

# gauss = [1, 6, 28, 89, 205, 338, 400, 338, 205, 89, 28, 6, 1]

r_buff = [0 for i in range(4)]	#4 pixels
g_buff = [0 for i in range(4)]
b_buff = [0 for i in range(4)]

horiz_r_buff = [0 for i in range(1921)]	#3 rows + 1 pixel
horiz_g_buff = [0 for i in range(1921)]
horiz_b_buff = [0 for i in range(1921)]

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

def input_buffer ():
	for i in range(3, 0, -1):
		r_buff[i] = r_buff[i-1]	#move items in delay line
		g_buff[i] = g_buff[i-1]
		b_buff[i] = b_buff[i-1]
	r_buff[0] = red		#new values come in
	g_buff[0] = green
	b_buff[0] = blue

def horiz_buffer ():
	for i in range(1920, 0, -1):
		horiz_r_buff[i] = horiz_r_buff[i-1]	#move rows in delay line
		horiz_g_buff[i] = horiz_g_buff[i-1]
		horiz_b_buff[i] = horiz_b_buff[i-1]
	horiz_r_buff[0] = r_blur_x	#writes to newest row of horiz
	horiz_g_buff[0] = g_blur_x
	horiz_b_buff[0] = b_blur_x


def avg_horiz ():
	global  r_sum_x, g_sum_x, b_sum_x, r_blur_x, g_blur_x, b_blur_x
	r_sum_x = r_buff[0] + r_buff[1] + r_buff[2] + r_buff[3]
	g_sum_x = g_buff[0] + g_buff[1] + g_buff[2] + g_buff[3]
	b_sum_x = b_buff[0] + b_buff[1] + b_buff[2] + b_buff[3]
	r_blur_x = r_sum_x / 4
	g_blur_x = g_sum_x / 4
	b_blur_x = b_sum_x / 4

def avg_vert ():
	global r_sum_y, g_sum_y, b_sum_y, r_blur_y, g_blur_y, b_blur_y
	r_sum_y = horiz_r_buff[0] + horiz_r_buff[640] + horiz_r_buff[1280] + horiz_r_buff[1920]
	g_sum_y = horiz_g_buff[0] + horiz_g_buff[640] + horiz_g_buff[1280] + horiz_g_buff[1920]
	b_sum_y = horiz_b_buff[0] + horiz_b_buff[640] + horiz_b_buff[1280] + horiz_b_buff[1920]
	r_blur_y = r_sum_y / 4
	g_blur_y = g_sum_y / 4
	b_blur_y = b_sum_y / 4

def post_assignments ():
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
			if (x==0): print(y)########
			avg_horiz()
			avg_vert()
			input_buffer()
			horiz_buffer()
			post_assignments()

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("out_blur.png")

print(f"Elapsed: {time.time() - start}")