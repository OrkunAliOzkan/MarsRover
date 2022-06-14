import time
import numpy as np
from PIL import Image

start = time.time()

img = Image.open('inputs/edge_test.png')
pixels = np.array(img)

n = 3
sobel_xx = [1, 0, -1]
#sobel_xx = [2, 1, 0, -1, -2]
#sobel_xx = [1, 1, 1, 0, -1, -1, -1]
#sobel_xx = [1, 1, 1, 1, 1, 1, 0, -1, -1, -1, -1, -1, -1]
# sobel_xy = [1, 2, 1]

edge_buff = [0 for i in range(n)]

# horiz_edge_buff = [0 for i in range(1281)]	# rows + 1 pixel

final = np.zeros(shape=(480, 640, 3))	#after all operations

edge_x = 0
#edge_y = 0


def pre_assignments ():
	global grey
	red = int(pixels[y, x, 0])	#aliases for input
	green = int(pixels[y, x, 1])
	blue = int(pixels[y, x, 2])
	grey = green/2 + red/4 + blue/4

def input_buffer ():
	for i in range(n-1, 0, -1):
		edge_buff[i] = edge_buff[i-1]	#move items in delay line
	edge_buff[0] = grey		#new values come in

# count_horiz = 0
# def horiz_buffer ():
# 	global count_horiz
# 	count_horiz = (count_horiz + 1) % (640*(n-1)+1)
# 	horiz_edge_buff[count_horiz] = edge_x

def edge_horiz ():
	global edge_x
	edge_x = 0
	for i in range(0, n, 1):
		edge_x = edge_x + sobel_xx[i]*edge_buff[i]
	edge_x = edge_x / ((n-1)/2)
	edge_x = abs(edge_x)
	#edge_x = 255 if edge_x > 100 else 0

# def edge_vert ():
# 	global edge_y
# 	edge_y = 0
# 	for i in range(0, n, 1):
# 		edge_y = edge_y + sobel_xy[i]*horiz_edge_buff[(count_horiz+(640*i)) % (640*(n-1)+1)]
# 	edge_y = edge_y / ((n+2)/2)
# 	edge_y = np.sqrt(edge_y**2)
# 	edge_y = 255 if edge_y > 70 else 0

def post_assignments ():
	final[y, x, 0] = edge_x	#writes to current row of final
	final[y, x, 1] = edge_x
	final[y, x, 2] = edge_x

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
			edge_horiz()
			#edge_vert()
			#if x==1: print(edge_x)
			input_buffer()
			#horiz_buffer()
			post_assignments()

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("results/out_edge.png")

print(f"Elapsed: {time.time() - start}")