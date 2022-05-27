import numpy as np
import random
import warnings
import keyboard
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.figure import Figure
from matplotlib.patches import Polygon

#parameters

arena_x = 360
arena_y = 240

aliens_count = 4
raycount = 20
ray_jump = 5 #initial ray distance for each point
ultrasound_jump = 1
alien_radius = 6
FOV = 70
friction = 1.1
acceleration = 0.2
max_vel = 10
turn_speed = 3
max_turn = 20
#walls_count = 40

running = True
manual = True
simulated = True

warnings.filterwarnings("ignore")
mpl.rcParams['toolbar'] = 'None'
plt.ion()

colours_list = ['lime', 'blue', 'magenta', 'yellow', 'red', 'cyan']

def sim_noise (input, uncertainty):
	return input*random.gauss(1, uncertainty)

def position_interface (): #translating world to internal model
	if simulated:
		vel = sim_noise(sim_rover.vel, 0.3)	# using drive input to estimate distance covered and steering
		return vel
	else:
		#real data goes here
		pass

def rotation_interface (): #translating world to internal model
	if simulated:
		turn = sim_noise(sim_rover.turn, 0.1)
		return turn
	else:
		#real data goes here
		pass

def vision_interface (): #receive camera data
	if simulated:
		viewing_noisy = []
		for i in range(len(viewing)):
			object = viewing[i][0]
			distance = viewing[i][1]
			angle = viewing[i][2]
			viewing_noisy.append([object, sim_noise(distance, 0.001*np.sqrt(distance)), 
								  sim_noise(angle, 0.0001*np.sqrt(angle))]) #angle is much more accurate than distance
		return viewing_noisy
	else:
		#real data goes here
		pass

class Ray ():

	def __init__ (self, rover_pos, rover_rot, angle, jump, map_num, type='ray'):
		self.position = rover_pos
		self.angle = angle
		self.theta = np.radians(rover_rot + self.angle)
		self.type = type
		self.jump = jump
		self.map_num = map_num
		self.coords = [[self.position[0]],[self.position[1]]]
		plt.figure(self.map_num)
		if self.type == 'ray':
			self.line, = plt.plot([self.coords[0][0], self.coords[0][-1]], #only plot first and last point
								   [self.coords[1][0], self.coords[1][-1]], color='lightblue', alpha=0.2, lw=3, zorder=1, marker='o')
		else: #ultrasound
			self.line, = plt.plot([self.coords[0][0], self.coords[0][-1]], #only plot first and last point
								   [self.coords[1][0], self.coords[1][-1]], color='blue', alpha=0.3, lw=5, zorder=1, marker='o')

	def ray_collide (self, head):
		point = np.asarray(head)
		for i, alien in enumerate(aliens):
			distance = np.linalg.norm(np.asarray(alien.position) - head)
			if distance < alien_radius:
				return alien
		if head[0] < 5:
			return walls[0] #left
		elif head[1] < 5:
			return walls[1]	#bottom
		elif head[1] > arena_y-5:
			return walls[2] #top
		elif head[0] > arena_x-5:
			return walls[3]	#right
		else:
			return None

	def cast (self, rover_pos, rover_rot):
		global viewing, aliens
		self.position = rover_pos
		self.theta = np.radians(rover_rot + self.angle)
		self.coords = [[self.position[0]],[self.position[1]]]
		dist = 0
		for i in range(int(100/self.jump)): #same distance for ray and ultrasound (US has smaller jumps)
			this_jump = self.jump * (1 + i/3)	#more inaccurate as distance increases
			dist += this_jump
			curr = [None, None]
			curr[0] = self.coords[0][-1] + this_jump*np.sin(self.theta)
			curr[1] = self.coords[1][-1] + this_jump*np.cos(self.theta)
			self.coords[0].append(curr[0])
			self.coords[1].append(curr[1])
			obj = self.ray_collide([self.coords[0][-1], self.coords[1][-1]])
			if obj != None:
				if type(obj) == Alien:
					viewing.append([obj, dist, self.angle])
				break
		plt.figure(self.map_num)
		self.line.set_data([[self.coords[0][0], self.coords[0][-1]], [self.coords[1][0], self.coords[1][-1]]])
		return dist

	def draw (self, rover_pos, rover_rot):
		self.position = rover_pos
		self.theta = np.radians(rover_rot + self.angle)
		self.coords = [[self.position[0]],[self.position[1]]]
		curr = [None, None]
		curr[0] = self.coords[0][-1] + self.jump*np.sin(self.theta)
		curr[1] = self.coords[1][-1] + self.jump*np.cos(self.theta)
		self.coords[0].append(curr[0])
		self.coords[1].append(curr[1])
		plt.figure(self.map_num)
		self.line.set_data([[self.coords[0][0], self.coords[0][-1]], [self.coords[1][0], self.coords[1][-1]]])
		
class Ultrasound (Ray):

	def __init__ (self, rover_pos, rover_rot, jump, map_num):
		super().__init__(rover_pos, rover_rot, 0, jump, map_num, 'ultrasound') #straight out the front
		self.reading_prev = 0
		self.reading = 0


# class Wall_Segment ():

# 	def __init__ (self, edge, i):
# 		self.edge = edge
# 		if self.edge == 0:
# 			self.position = [3, (i%6)*50] #[x, y]
# 		elif self.edge == 1:
# 			self.position = [(i%9)*50, 3]
# 		elif self.edge == 2:
# 			self.position = [(i%9)*50, 242]
# 		elif self.edge == 3:
# 			self.position = [362, (i%6)*50]
# 		if self.edge == 1 or self.edge == 2:
# 			self.shape = patches.Rectangle([self.position[0] - 60, self.position[1] - 5], 60, 5, color="gray", zorder=1)
# 		else:
# 			self.shape = patches.Rectangle([self.position[0] - 5, self.position[1] - 50], 5, 50, color="gray", zorder=1)

class Wall ():

	def __init__ (self, edge):
		self.edge = edge
		self.coords = [[],[]]
		if self.edge == 0:	#left
			self.coords[0] = [0, 0] #x
			self.coords[1] = [0, arena_y]	#y
		elif self.edge == 1:	#bottom
			self.coords[0] = [0, arena_x] #x
			self.coords[1] = [0, 0]	#y
		elif self.edge == 2:	#top
			self.coords[0] = [0, arena_x] #x
			self.coords[1] = [arena_y, arena_y]	#y
		elif self.edge == 3:	#right
			self.coords[0] = [arena_x, arena_x] #x
			self.coords[1] = [0, arena_y]	#y
		if simulated:
			plt.figure(2)
			self.line, = plt.plot([self.coords[0][0], self.coords[0][-1]], #only plot first and last point
								   [self.coords[1][0], self.coords[1][-1]], color='gray', lw=7, zorder=1)
		plt.figure(1)
		self.line, = plt.plot([self.coords[0][0], self.coords[0][-1]], #only plot first and last point
								   [self.coords[1][0], self.coords[1][-1]], color='gray', lw=7, zorder=1)

class Alien ():

	def __init__ (self, i):
		self.quadrant = i % 4 #realistic placement
		if self.quadrant == 0:
			self.position = [random.randint(20, arena_x/2), random.randint(20, arena_y/2)] #[x, y]
		elif self.quadrant == 1:
			self.position = [random.randint(arena_x/2, arena_x-20), random.randint(20, arena_y/2)]
		elif self.quadrant == 2:
			self.position = [random.randint(20, arena_x/2), random.randint(arena_y/2, arena_y-20)]
		elif self.quadrant == 3:
			self.position = [random.randint(arena_x/2, arena_x-20), random.randint(arena_y/2, arena_y-20)]
		self.colour = colours_list[i]
		plt.figure(2)
		self.scatter = plt.scatter(self.position[0], self.position[1], color=self.colour, alpha=1, zorder=3)
		plt.figure(1)
		self.scatter = plt.scatter(self.position[0], self.position[1], color=self.colour, alpha=0, zorder=3)

class Rover ():

	def __init__ (self):
		self.vel = 0 	#velocity accumulated
		self.turn = 0 	#turning accumulated
		self.edge = 0 ##########
		self.position = [25, 25]
		# self.edge = random.randint(0, 3) #irl the rover would be placed on one of the edges or corners
		# if self.edge == 0:
		# 	self.position = [random.randint(10, 40), random.randint(10, 230)] #[x, y]
		# elif self.edge == 1:
		# 	self.position = [random.randint(10, 350), random.randint(10, 40)]
		# elif self.edge == 2:
		# 	self.position = [random.randint(10, 350), random.randint(190, 230)]
		# elif self.edge == 3:
		# 	self.position = [random.randint(310, 350), random.randint(10, 230)]
		self.coords = [[self.position[0] - 9, self.position[1] - 10],	#dimensions of the rover
					   [self.position[0] + 9, self.position[1] - 10],
					   [self.position[0] + 9, self.position[1] + 10],
					   [self.position[0] - 9, self.position[1] + 10]]
		self.rotation = 0 #############
		#self.rotation = random.randint(0, 360) 	#intial random rotation
		#for i in range(4):
		#	self.coords[i] = self.rotate(self.coords[i], self.rotation)
		self.shape = Polygon(np.asarray(self.coords), closed=False, color="gray", alpha=0.8, zorder=2)

	def rotate (self, point, dr):	#for rotating each point on the shape about the rover centre
		theta = np.radians(dr)
		point0 = self.position[0] + np.cos(theta) * (point[0] - self.position[0]) + np.sin(theta) * (point[1] - self.position[1])
		point1 = self.position[1] - np.sin(theta) * (point[0] - self.position[0]) + np.cos(theta) * (point[1] - self.position[1])
		return [point0, point1]

	def translate (self, point, dd):	#for updating each point in the shape
		point0 = point[0] + dd * np.sin(np.radians(self.rotation))
		point1 = point[1] + dd * np.cos(np.radians(self.rotation))
		return [point0, point1]

	def update (self, dd, dr):
		dx = dd * np.sin(np.radians(self.rotation)) #h*sin(theta)
		dy = dd * np.cos(np.radians(self.rotation))
		self.position[0] += dx
		self.position[1] += dy
		for i in range(4):
			self.coords[i] = self.translate(self.coords[i], dd)
		if dr != 0:
			self.rotation += dr
			for i in range(4):
				self.coords[i] = self.rotate(self.coords[i], dr)
		self.shape.set_xy(self.coords) #update shape

	def movement (self):	#allows for simulated acceleration and friction
		if keyboard.is_pressed('w') or keyboard.is_pressed('up arrow'):
			self.vel = self.vel + acceleration if self.vel < max_vel else max_vel 		#capped velocity
		elif keyboard.is_pressed('s') or keyboard.is_pressed('down arrow'):
			self.vel = self.vel - acceleration if self.vel > -max_vel else -max_vel
		else:
			self.vel = self.vel / friction if abs(self.vel) > 0.05 else 0
		if keyboard.is_pressed('a') or keyboard.is_pressed('left arrow'):
			if self.vel >= 0:
				self.turn = self.turn - turn_speed if self.turn > -max_turn else -max_turn
			else:
				self.turn = self.turn + turn_speed if self.turn < max_turn else max_turn 	# reverse turn direction when reversing
		elif keyboard.is_pressed('d') or keyboard.is_pressed('right arrow'):
			if self.vel >= 0:
				self.turn = self.turn + turn_speed if self.turn < max_turn else max_turn
			else:
				self.turn = self.turn - turn_speed if self.turn > -max_turn else -max_turn
		else:
			self.turn = self.turn / (friction+0.3) if abs(self.turn) > 0.01 else 0
		self.update(self.vel, self.turn) #vel is just delta distance

	def physical_position_rotation (self): #drive input doesn't exactly translate to output
		vel = sim_noise(rover.vel, 0.2 + 0.01*rover.vel) #uncertainty depends on speed
		turn = sim_noise(rover.turn, 0.05 + 0.01*rover.turn)
		self.update(vel, turn)

	def estimate_position_rotation (self): #attempt corrections
		dd = position_interface()
		dr = rotation_interface()
		self.update(dd, dr)
		pass

	def vision (self): #get data from camera
		self.viewing = vision_interface()
		for i in range(len(self.viewing)):
			object = self.viewing[i][0]
			distance = self.viewing[i][1]
			angle = self.viewing[i][2]
			theta = angle + self.rotation
			pos_x = self.position[0] + distance * np.sin(np.radians(theta)) #rover pos + alien dist
			pos_y = self.position[1] + distance * np.cos(np.radians(theta))
			object.scatter.set_offsets([pos_x, pos_y])
			object.scatter.set_alpha(1)			#make viewing objects visible
		self.viewing = []

class Map ():

	def __init__ (self, title, num):
		global fig
		fig = plt.figure(num, figsize=(arena_x/50, arena_y/50))
		plt.style.use('Solarize_Light2')
		if num == 2: plt.rcParams.update({'axes.facecolor':'sandybrown'})	#orange background for sim
		plt.title(title)
		plt.xlim([0, arena_x])
		plt.ylim([0, arena_y])
		plt.gca().set_aspect('equal', adjustable='box') #never stretch axes
		plt.grid(color='lightgray', lw=0.5)

	def map_axis (self):
		global ax1
		plt.figure(1)
		ax1 = plt.subplot(111)

	def sim_axis (self):
		global ax2
		plt.figure(2)
		ax2 = plt.subplot(111)

def clear ():
	try:
		plt.close()
	except Exception as e:
		print(e)


def run ():
	global sim_map, arena_map, sim_rover, rover, ax1, ax2, viewing, aliens, walls
	rover = Rover()
	arena_map = Map("Arena Map", 1)
	arena_map.map_axis()
	if simulated:
		sim_rover = Rover()		#instantiations
		sim_map = Map("Simulated Map", 2)	#'real world'
		sim_map.sim_axis()
		aliens = []
		for i in range(aliens_count):
			aliens.append(Alien(i))
		walls = []
		for edge in range(4):
			walls.append(Wall(edge))
		rays = []
		for i in range(raycount):
			angle = (FOV/(raycount+1))*(i+1) - FOV/2 	#spread rays
			rays.append(Ray(rover.position, rover.rotation, angle, ray_jump, 2))
		ultrasound = Ultrasound(rover.position, rover.rotation, ultrasound_jump, 2)
		ax2.add_patch(sim_rover.shape)
	camera_visual = []	#visual FOV representation
	camera_visual.append(Ray(rover.position, rover.rotation, -FOV/2, 450, 1))
	camera_visual.append(Ray(rover.position, rover.rotation, FOV/2, 450, 1))
	ax1.add_patch(rover.shape)
	plt.show()	#display plot
	count = 0 	#for infrequent subroutines
	while running:
		##subroutines
		viewing = []	#list of objects in cone of vision
		rover.movement()
		if simulated:	#keyboard control
			sim_rover.physical_position_rotation()
			for i, ray in enumerate(rays):
				ray.cast(sim_rover.position, sim_rover.rotation)	#shoot rays until object
				us_distance = ultrasound.cast(sim_rover.position, sim_rover.rotation)######
		#rover.estimate_position_rotation()
		for i in range(2):
			camera_visual[i].draw(rover.position, rover.rotation)
		rover.vision()
		plt.pause(0.001)			#update plot
		if keyboard.is_pressed('escape'):
			clear()
			quit()
		count += 1


if __name__ == "__main__":	#only run if not imported
	run()