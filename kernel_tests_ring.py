import numpy as np
from PIL import Image

img = Image.open('image.png')
pixels = np.array(img)

gauss = [1, 6, 15, 20, 15, 6, 1]

r_ring = [0 for i in range(7)]		#7 pixels
g_ring = [0 for i in range(7)]
b_ring = [0 for i in range(7)]
count_input = 0

horiz_ring_r = np.zeros(shape=(3841))	#6 rows + 1 pixel
horiz_ring_g = np.zeros(shape=(3841))
horiz_ring_b = np.zeros(shape=(3841))
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
	count_input = (count_input + 1) % 7
	r_ring[count_input] = red		#new values come in
	g_ring[count_input] = green
	b_ring[count_input] = blue

def horiz_ring ():
	global count_horiz
	count_horiz = (count_horiz + 1) % 3841

def avg_horiz ():
	global r_blur_x, g_blur_x, b_blur_x
	r_sum_x = gauss[0]*r_ring[(count_input+1)%7] + gauss[1]*r_ring[(count_input+2)%7] + gauss[2]*r_ring[(count_input+3)%7] + gauss[3]*r_ring[(count_input+4)%7] + gauss[4]*r_ring[(count_input+5)%7] + gauss[5]*r_ring[(count_input+6)%7] + gauss[6]*r_ring[(count_input+7)%7]
	g_sum_x = gauss[0]*g_ring[(count_input+1)%7] + gauss[1]*g_ring[(count_input+2)%7] + gauss[2]*g_ring[(count_input+3)%7] + gauss[3]*g_ring[(count_input+4)%7] + gauss[4]*g_ring[(count_input+5)%7] + gauss[5]*g_ring[(count_input+6)%7] + gauss[6]*g_ring[(count_input+7)%7]
	b_sum_x = gauss[0]*b_ring[(count_input+1)%7] + gauss[1]*b_ring[(count_input+2)%7] + gauss[2]*b_ring[(count_input+3)%7] + gauss[3]*b_ring[(count_input+4)%7] + gauss[4]*b_ring[(count_input+5)%7] + gauss[5]*b_ring[(count_input+6)%7] + gauss[6]*b_ring[(count_input+7)%7]
	r_blur_x = r_sum_x / 64
	g_blur_x = g_sum_x / 64
	b_blur_x = b_sum_x / 64

def avg_vert ():
	global r_sum_y, g_sum_y, b_sum_y, r_blur_y, g_blur_y, b_blur_y
	r_sum_y = gauss[0]*horiz_ring_r[(count_horiz+1)%3841] + gauss[1]*horiz_ring_r[(count_horiz+641)%3841] + gauss[2]*horiz_ring_r[(count_horiz+1281)%3841] + gauss[3]*horiz_ring_r[(count_horiz+1921)%3841] + gauss[4]*horiz_ring_r[(count_horiz+2561)%3841] + gauss[5]*horiz_ring_r[(count_horiz+3201)%3841] + gauss[6]*horiz_ring_r[(count_horiz+3841)%3841]
	g_sum_y = gauss[0]*horiz_ring_g[(count_horiz+1)%3841] + gauss[1]*horiz_ring_g[(count_horiz+641)%3841] + gauss[2]*horiz_ring_g[(count_horiz+1281)%3841] + gauss[3]*horiz_ring_g[(count_horiz+1921)%3841] + gauss[4]*horiz_ring_g[(count_horiz+2561)%3841] + gauss[5]*horiz_ring_g[(count_horiz+3201)%3841] + gauss[6]*horiz_ring_g[(count_horiz+3841)%3841]
	b_sum_y = gauss[0]*horiz_ring_b[(count_horiz+1)%3841] + gauss[1]*horiz_ring_b[(count_horiz+641)%3841] + gauss[2]*horiz_ring_b[(count_horiz+1281)%3841] + gauss[3]*horiz_ring_b[(count_horiz+1921)%3841] + gauss[4]*horiz_ring_b[(count_horiz+2561)%3841] + gauss[5]*horiz_ring_b[(count_horiz+3201)%3841] + gauss[6]*horiz_ring_b[(count_horiz+3841)%3841]
	r_blur_y = r_sum_y / 64
	g_blur_y = g_sum_y / 64
	b_blur_y = b_sum_y / 64

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