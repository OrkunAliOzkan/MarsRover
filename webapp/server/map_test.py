from cv2 import magnitude
import numpy as np
import os
import sys

from timeit import default_timer as timer

def waypoint_cmd(rx, ry, wx, wy):
    dx = wx - rx
    dy = wy - ry
    magnitude = np.sqrt(dx ** 2 + dy ** 2)
    angle = np.arctan(dy/dx)
    return (magnitude, angle)

print("hello world")
# f = open("demo.txt", "a")
# f.write("hello world\n")
# f.close()

# count = 0
# while count < 3: 
#     start = 
#     f.write("hello world\n")
# f.close()

# while True:
#     sys.stdin.read()