import numpy as np
import random
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.figure import Figure

arena_x = 360
arena_y = 240

mpl.rcParams['toolbar'] = 'None'

def full_map ():
	full_map = Figure(figsize=(7.2, 10.8))
	full_map.canvas.set_window_title('Simulated Map')
	plt.ylim([0, 240])
	plt.xlim([0, 360])
	plt.style.use('Solarize_Light2')
	plt.grid(color='lightgray', lw=0.5)
	dot = plt.scatter(0, 0, color='firebrick') ########
	plt.show()

def random_test ():

full_map()