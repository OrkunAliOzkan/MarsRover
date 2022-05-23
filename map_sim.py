import numpy as np
import random
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.figure import Figure

arena_x = 360
arena_y = 240

running = True

mpl.rcParams['toolbar'] = 'None'
plt.ion()

class Map ():

	def __init__ (self):
		global map_fig
		map_fig = Figure(figsize=(7.2, 10.8))
		plt.style.use('Solarize_Light2')
		plt.title('Simulated Map')
		plt.ylim([0, 240])
		plt.xlim([0, 360])
		plt.grid(color='lightgray', lw=0.5)
		self.dot = plt.scatter(0, 0, color='firebrick') ########
		plt.show()

def clear ():
	try:
		plt.close()
	except:
		pass
	try:
		map.__del__(self)
		map_fig.__del__(self)
	except:
		pass

def plot ():
	global sim_map
	sim_map.dot.set_offsets([np.random.randint(1, 360), np.random.randint(1, 240)]) #####
	plt.pause(0.001)

def run ():
	global sim_map
	clear()
	sim_map = Map()
	while running:
		##subroutines
		plot()


if __name__ == "__main__":
	run()