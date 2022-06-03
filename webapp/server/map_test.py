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

# buff = ''
# while True:
#     ch = sys.stdin.read(1)
#     if ch == '\n':
#         if buff == "q": break
#         print(buff)
#         buff = ''
#     else:
#         buff += ch

for line in sys.stdin:
    if line.rstrip() == "q": 
        break
    print("received\n")

# f = open("out.txt", "a")
# f.write("helloworld\n")
# for line in sys.stdin:
#     if line == "q": 
#         break
#     sys.stdout.write("received \n")
# f.close()