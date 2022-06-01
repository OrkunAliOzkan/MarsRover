from cv2 import magnitude
import numpy as np
import os

def waypoint_cmd(rx, ry, wx, wy):
    dx = wx - rx
    dy = wy - ry
    magnitude = np.sqrt(dx ** 2 + dy ** 2)
    angle = np.arctan(dy/dx)
    return (magnitude, angle)
