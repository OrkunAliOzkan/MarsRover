import numpy as np
import random
import warnings
import keyboard
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.figure import Figure
from matplotlib.patches import Polygon

arena_x = 360
arena_y = 240

aliens_count = random.randint(3, 6)
walls_count = 40
raycount = 20
FOV = 70

running = True
manual = True

warnings.filterwarnings("ignore")
plt.gca().set_aspect('equal', adjustable='box')
mpl.rcParams['toolbar'] = 'None'
plt.ion()

colours_list = ['red', 'lime', 'blue', 'magenta', 'yellow', 'cyan']

class Ray ():

	def __init__ (self, rover_pos, rover_rot, angle):
		self.position = rover_pos
		self.angle = angle
		self.theta = np.radians(rover_rot + self.angle)
		self.jump = 10
		self.line, = plt.plot([self.position[0], self.position[1]], color='lightblue', alpha=0.2, lw=3, zorder=1)

	def ray_collide (self, head):
		point = np.asarray(head)
		for i, alien in enumerate(aliens):
			distance = np.linalg.norm(np.asarray(alien.position) - head)
			if distance < 4:
				return alien

	def cast (self, rover_pos, rover_rot):
		global viewing
		self.position = rover_pos
		self.theta = np.radians(rover_rot + self.angle)
		self.coords = [[self.position[0]],[self.position[1]]]
		for i in range(45):
			curr = [None, None]
			curr[0] = self.coords[0][-1] + self.jump*np.sin(self.theta)
			curr[1] = self.coords[1][-1] + self.jump*np.cos(self.theta)
			self.coords[0].append(curr[0])
			self.coords[1].append(curr[1])
			obj = self.ray_collide([self.coords[0][-1], self.coords[1][-1]])
			if obj != None:
				viewing.append(obj)
				break
		self.line.set_data([self.coords[0], self.coords[1]])

class Wall_segment ():

	def __init__ (self, edge, i):
		self.edge = edge
		if self.edge == 0:
			self.position = [3, (i%6)*50] #[x, y]
		elif self.edge == 1:
			self.position = [(i%9)*50, 3]
		elif self.edge == 2:
			self.position = [(i%9)*50, 242]
		elif self.edge == 3:
			self.position = [362, (i%6)*50]
		if self.edge == 1 or self.edge == 2:
			self.shape = patches.Rectangle([self.position[0] - 60, self.position[1] - 5], 60, 5, color="gray", zorder=1)
		else:
			self.shape = patches.Rectangle([self.position[0] - 5, self.position[1] - 50], 5, 50, color="gray", zorder=1)

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
		self.scatter = plt.scatter(self.position[0], self.position[1], color=self.colour, alpha=0, zorder=3)

class Rover ():

	def __init__ (self):
		self.edge = random.randint(0, 3) #irl the rover would be placed on one of the edges or corners
		if self.edge == 0:
			self.position = [random.randint(10, 40), random.randint(10, 230)] #[x, y]
		elif self.edge == 1:
			self.position = [random.randint(10, 350), random.randint(10, 40)]
		elif self.edge == 2:
			self.position = [random.randint(10, 350), random.randint(190, 230)]
		elif self.edge == 3:
			self.position = [random.randint(310, 350), random.randint(10, 230)]
		self.coords = [[self.position[0] - 9, self.position[1] - 10],
					   [self.position[0] + 9, self.position[1] - 10],
					   [self.position[0] + 9, self.position[1] + 10],
					   [self.position[0] - 9, self.position[1] + 10]]
		self.shape = Polygon(np.asarray(self.coords), closed=False, color="gray", alpha=0.8, zorder=2)
		self.rotation = 0

	def rotate (self, point, dr):
		theta = np.radians(dr)
		point0 = self.position[0] + np.cos(theta) * (point[0] - self.position[0]) + np.sin(theta) * (point[1] - self.position[1])
		point1 = self.position[1] - np.sin(theta) * (point[0] - self.position[0]) + np.cos(theta) * (point[1] - self.position[1])
		return [point0, point1]

	def translate (self, point, dd):
		point0 = point[0] + dd * np.sin(np.radians(self.rotation))
		point1 = point[1] + dd * np.cos(np.radians(self.rotation))
		return [point0, point1]

	def update (self, dd, dr):
		dx = dd * np.sin(np.radians(self.rotation))
		dy = dd * np.cos(np.radians(self.rotation))
		self.position[0] += dx
		self.position[1] += dy
		for i in range(4):
			self.coords[i] = self.translate(self.coords[i], dd)
		if dr != 0:
			self.rotation += dr
			for i in range(4):
				self.coords[i] = self.rotate(self.coords[i], dr)
		self.shape = Polygon(np.asarray(self.coords), closed=False, color="gray", alpha=0.8, zorder=2)


class Map ():

	def __init__ (self):
		global map_fig, ax
		map_fig = plt.figure(figsize=(7.2, 4.8))
		plt.style.use('Solarize_Light2')
		plt.rcParams.update({'axes.facecolor':'sandybrown'})
		plt.title('Simulated Map')
		plt.ylim([0, 240])
		plt.xlim([0, 360])
		ax = plt.subplot(111)
		plt.grid(color='lightgray', lw=0.5)

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
	global sim_map, rover, ax, viewing, aliens
	clear()
	rover = Rover()
	sim_map = Map()
	aliens = []
	for i in range(aliens_count):
		aliens.append(Alien(i))
	walls = []
	for edge in range(4):
		for i in range(int(walls_count/4)):
			walls.append(Wall_segment(edge, i))
	for i, wall in enumerate(walls):
		ax.add_patch(wall.shape)
	rays = []
	for i in range(raycount):
		angle = (FOV/(raycount+1))*(i+1) - FOV/2
		rays.append(Ray(rover.position, rover.rotation, angle))
	plt.show()
	while running:
		##subroutines
		if manual:
			keys()
		try: ax.patches.pop()
		except: pass
		viewing = []
		for i, ray in enumerate(rays):
			ray.cast(rover.position, rover.rotation)
		for i, object in enumerate(viewing):
			object.scatter.set_alpha(1)
		ax.add_patch(rover.shape)
		plt.pause(0.001)


if __name__ == "__main__":
	run()