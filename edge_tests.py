import time
import numpy as np
from PIL import Image

start = time.time()

img = Image.open('inputs/mars8.png')
pixels = np.array(img)

n = 3
sobel_xx = [1, 0, -1]
#sobel_xx = [2, 1, 0, -1, -2]
#sobel_xx = [1, 1, 1, 0, -1, -1, -1]
#sobel_xx = [1, 1, 1, 1, 1, 1, 0, -1, -1, -1, -1, -1, -1]
# sobel_xy = [1, 2, 1]

edge_buff = [0 for i in range(n)]
Sat_buff = [0 for i in range(n-1)]
Val_buff = [0 for i in range(n-1)]

# horiz_edge_buff = [0 for i in range(1281)]	# rows + 1 pixel

final = np.zeros(shape=(480, 640, 3))	#after all operations

edge_x = 0
#edge_y = 0

def pre_assignments ():
	global red, green, blue, grey, edge_detected, face_detected
	red = int(pixels[y, x, 0])	#aliases for input
	green = int(pixels[y, x, 1])
	blue = int(pixels[y, x, 2])
	grey = green/2 + red/4 + blue/4
	edge_detected = False
	face_detected = False

def input_buffer ():
	for i in range(n-1, 0, -1):
		edge_buff[i] = edge_buff[i-1]	#move items in delay line
	edge_buff[0] = grey		#new values come in

def col_buffers ():
	Sat_buff[1] = Sat_buff[0]	#move items in delay line
	Val_buff[1] = Val_buff[0]
	Sat_buff[0] = Sat		#new values come in
	Val_buff[0] = Val

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
	edge_x = 255 if edge_x > 40 else 0

def HSV_conversion ():
	global Sat, Val
	RGB_max = red if (red > green and red > blue) else green if (green > blue) else blue
	RGB_min = red if (red < green and red < blue) else green if (green < blue) else blue
	RGB_diff = RGB_max - RGB_min if (RGB_max-RGB_min != 0) else 1
	Val = RGB_max
	Sat = 0 if Val == 0 else (100 * RGB_diff) / RGB_max

edge_prev = 0	#stores old high value
measured_list = [0 for i in range(30)]	#stores valid edges belonging to a building
measured_count = 0
faces_list = [0 for i in range(30)]	#stores valid faces belonging to a building
faces_count = 0
first_face_black = 0 #to check alternating pattern

def edge_measure ():
	global edge_prev, edge_detected, face_detected, measured_count, faces_count, first_face_black
	if (y == 240):	#middle row
		if (x > 64 and x < 576):	#exclude sides of image
			if (edge_x == 255):		#check if edge at all
				if edge_prev == 0:	#first edge
					measured_list[measured_count] = x
					edge_detected = True
					measured_count += 1
				elif (x - edge_prev > 2):	#small gap rejection
					if (x - edge_prev < 150):	#large gap rejection
						measured_list[measured_count] = x
						edge_detected = True
						measured_count += 1
				edge_prev = x 		#next edge prev if 255
	if edge_detected == True:	#now looking for faces
		if measured_count > 2:	#reject edges of cylinder
			if Sat_buff[1] < 100: #Check 2 pixels behind in case coloured ball (leeway for fuzzy edges)
				if faces_list[0] == 0:	#no previous faces
					faces_list[faces_count] = x-2
					first_face_black = 1 if Val_buff[1] < 100 else 0
					face_detected = True
					faces_count += 1
				else:	#previous faces
					if ((faces_count+first_face_black)%2 == (Val_buff[1] < 100)): #all three True or False
						faces_list[faces_count] = x-2
						face_detected = True
						faces_count += 1


# def edge_vert ():
# 	global edge_y
# 	edge_y = 0
# 	for i in range(0, n, 1):
# 		edge_y = edge_y + sobel_xy[i]*horiz_edge_buff[(count_horiz+(640*i)) % (640*(n-1)+1)]
# 	edge_y = edge_y / ((n+2)/2)
# 	edge_y = np.sqrt(edge_y**2)
# 	edge_y = 255 if edge_y > 70 else 0

def post_assignments ():
	# final[y, x, 0] = 255 if face_detected else 0 if edge_detected else edge_x	#writes to current row of final
	# final[y, x, 1] = 255 if face_detected else 0 if edge_detected else edge_x
	# final[y, x, 2] = 0 if face_detected else 255 if edge_detected else edge_x
	final[y, x, 0] = edge_x
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
			HSV_conversion()
			edge_measure()
			input_buffer()
			#horiz_buffer()
			post_assignments()
			col_buffers()

faces_list[faces_count-1] = 0

for x in measured_list:
	if x != 0:
		final[:, x, 0] = 0
		final[:, x, 1] = 100
		final[:, x, 2] = 255

for x in faces_list:
	if x != 0:
		final[:, x, 0] = 255
		final[:, x, 1] = 200
		final[:, x, 2] = 0

out = Image.fromarray(np.asarray(final).astype(np.uint8))
out.save("results/out_edge.png")

print(f"Elapsed: {time.time() - start}")