import numpy as np
import os
import sys

from timeit import default_timer as timer
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