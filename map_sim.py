import numpy as np
import random
import warnings
import keyboard
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.patches import Polygon

arena_x = 360
arena_y = 240

running = True
manual = True

warnings.filterwarnings("ignore")
plt.gca().set_aspect('equal', adjustable='box')
mpl.rcParams['toolbar'] = 'None'
plt.ion()

colours_list = ['red', 'lime', 'blue', 'magenta', 'yellow', 'cyan']

class Alien ():

	def __init__ (self, i):
		self.quadrant = i % 4 #realistic placement
		if self.quadrant == 0:
			self.position = [random.randint(20, 180), random.randint(20, 120)] #[x, y]
		elif self.quadrant == 1:
			self.position = [random.randint(180, 340), random.randint(20, 120)]
		elif self.quadrant == 2:
			self.position = [random.randint(20, 180), random.randint(120, 220)]
		elif self.quadrant == 3:
			self.position = [random.randint(180, 340), random.randint(120, 220)]
		self.colour = colours_list[i]
		self.scatter = plt.scatter(self.position[0], self.position[1], color=self.colour)

class Rover ():

	def __init__ (self):
		edge = random.randint(0, 3) #irl the rover would be placed on one of the edges or corners
		if edge == 0:
			self.position = [random.randint(10, 40), random.randint(10, 230)] #[x, y]
		elif edge == 1:
			self.position = [random.randint(10, 350), random.randint(10, 40)]
		elif edge == 2:
			self.position = [random.randint(10, 350), random.randint(190, 230)]
		elif edge == 3:
			self.position = [random.randint(310, 350), random.randint(10, 230)]
		self.coords = [[self.position[0] - 4, self.position[1] - 5],
					   [self.position[0] + 4, self.position[1] - 5],
					   [self.position[0] + 4, self.position[1] + 5],
					   [self.position[0] - 4, self.position[1] + 5]]
		self.shape = Polygon(np.asarray(self.coords), closed=False, color="gray", alpha=0.6)
		self.rotation = 0

	def rotate (self, point, dr):
		theta = np.radians(dr)
		point0 = self.position[0] + np.cos(theta) * (point[0] - self.position[0]) + np.sin(theta) * (point[1] - self.position[1])
		point1 = self.position[1] - np.sin(theta) * (point[0] - self.position[0]) + np.cos(theta) * (point[1] - self.position[1])
		return [point0, point1]

	def translate (self, point, dd):
		if dd > 0:
			point0 = point[0] + dd * np.sin(np.radians(self.rotation))
			point1 = point[1] + dd * np.cos(np.radians(self.rotation))
		else:
			point0 = point[0] - dd * np.sin(np.radians(self.rotation))
			point1 = point[1] - dd * np.cos(np.radians(self.rotation))
		return [point0, point1]

	def update (self, dd, dr):
		if dd > 0:
			dx = dd * np.sin(np.radians(self.rotation))
			dy = dd * np.cos(np.radians(self.rotation))
			self.position[0] += dx
			self.position[1] += dy
			for i in range(4):
				self.coords[i] = self.translate(self.coords[i], dd)
		elif dd < 0:
			dx = dd * np.sin(np.radians(self.rotation))
			dy = dd * np.cos(np.radians(self.rotation))
			self.position[0] -= dx
			self.position[1] -= dy
			for i in range(4):
				self.coords[i] = self.translate(self.coords[i], dd)
		if dr != 0:
			self.rotation += dr
			for i in range(4):
				self.coords[i] = self.rotate(self.coords[i], dr)
		self.shape = Polygon(np.asarray(self.coords), closed=False, color="gray", alpha=0.6)


class Map ():

	def __init__ (self):
		global map_fig, ax
		map_fig = plt.figure(figsize=(7.2, 4.8))
		plt.style.use('Solarize_Light2')
		plt.rcParams.update({'axes.facecolor':'sandybrown'})
		plt.title('Simulated Map')
		plt.ylim([0, arena_y])
		plt.xlim([0, arena_x])
		ax = plt.subplot(111)
		plt.grid(color='lightgray', lw=0.5)
		self.aliens_count = random.randint(3, 6)
		self.aliens = []
		for i in range(self.aliens_count):
			self.aliens.append(Alien(i))
		plt.show()

def keys ():
	if keyboard.is_pressed('w') or keyboard.is_pressed('up arrow'):
		rover.update(2, 0)
	if keyboard.is_pressed('a') or keyboard.is_pressed('left arrow'):
		rover.update(0, -10)
	if keyboard.is_pressed('s') or keyboard.is_pressed('down arrow'):
		rover.update(-2, 0)
	if keyboard.is_pressed('d') or keyboard.is_pressed('right arrow'):
		rover.update(0, 10)

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


def run ():
	global sim_map, rover, ax
	clear()
	rover = Rover()
	sim_map = Map()
	while running:
		##subroutines
		if manual:
			keys()
		try: ax.patches = []
		except: pass
		ax.add_patch(rover.shape)
		plt.pause(0.001)



if __name__ == "__main__":
	run()