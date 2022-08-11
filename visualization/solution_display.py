import matplotlib.image as mpimg
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
from PIL import Image, ImageOps
import pydot
import random
import time

LEFT = 0
STAY = 1
RIGHT = 2
COMPOUND = 3
LOOP = 4

NODE_STATE_NONE = 0
NODE_STATE_NEW = 1
NODE_STATE_NEW_CHILD = 2
NODE_STATE_ON_PATH = 3
NODE_STATE_RANDOM = 4
NODE_STATE_GOOD_EXPLORE = 5
NODE_STATE_RANDOM_EXPLORE = 6
NODE_STATE_FORCED_EXPLORE = 7

CHILD_PATH_STATE_NONE = 0
CHILD_PATH_STATE_NEW = 1
CHILD_PATH_STATE_PROCESSED = 2
CHILD_PATH_STATE_SELECTED = 3
CHILD_PATH_STATE_RANDOM = 4

def pretty_print_action(action):
	if action[1] == -1:
		return ''

	if action[1] == COMPOUND:
		return 'C ' + str(action[2])

	if action[1] == LOOP:
		return 'L ' + str(action[2])

	result = '('
	result += "{:.2f}".format(action[0])
	result += ', '
	if action[1] == 0:
		result += 'LEFT'
	elif action[1] == 1:
		result += 'STAY'
	else:
		result += 'RIGHT'
	result += ')'
	return result

mplstyle.use('fast')
plt.ion()
fig, ax = plt.subplots()
plt.margins(x=0, y=0)

while True:
	graph = pydot.Dot(graph_type='digraph', strict=True)

	nodes = []
	with open('../display.txt') as file:
		num_nodes = int(file.readline())
		for n_index in range(num_nodes):
			state = NODE_STATE_NONE
			children_indexes = []
			children_actions = []
			process_information = []

			num_children = int(file.readline())
			for c_index in range(num_children):
				child_index = int(file.readline())
				children_indexes.append(child_index)
				
				write = float(file.readline())
				move = int(file.readline())
				index = int(file.readline())
				children_actions.append([write, move, index])

				child_path_state = CHILD_PATH_STATE_NONE
				predicted_score = 0.0
				predicted_misguess = 0.0
				process_information.append(
					[child_path_state,
					 predicted_score,
					 predicted_misguess])

			average_score = file.readline().strip()

			nodes.append([state,
						  children_indexes,
						  children_actions,
						  process_information,
						  average_score])

		while True:
			curr_node_index = int(file.readline())
			if curr_node_index == 0:
				break
			elif curr_node_index == -1:
				break

			next_line = file.readline().strip()
			if next_line == 'new_node':
				nodes[curr_node_index][0] = NODE_STATE_NEW
				nodes[curr_node_index][3][0][0] = CHILD_PATH_STATE_NEW
			elif next_line == 'new_child':
				child_index = int(file.readline())

				nodes[curr_node_index][0] = NODE_STATE_NEW_CHILD
				nodes[curr_node_index][3][child_index][0] = CHILD_PATH_STATE_NEW
			elif next_line == 'random':
				child_index = int(file.readline())

				nodes[curr_node_index][0] = NODE_STATE_RANDOM
				nodes[curr_node_index][3][child_index][0] = CHILD_PATH_STATE_RANDOM
			elif next_line == 'good_explore':
				nodes[curr_node_index][0] = NODE_STATE_GOOD_EXPLORE
			elif next_line == 'random_explore':
				nodes[curr_node_index][0] = NODE_STATE_RANDOM_EXPLORE
			elif next_line == 'forced_explore':
				nodes[curr_node_index][0] = NODE_STATE_FORCED_EXPLORE
			else:
				nodes[curr_node_index][0] = NODE_STATE_ON_PATH
				for c_index in range(len(nodes[curr_node_index][1])):
					predicted_score = float(file.readline())
					predicted_misguess = float(file.readline())
					nodes[curr_node_index][3][c_index] = [
						CHILD_PATH_STATE_PROCESSED,
						predicted_score,
						predicted_misguess]

				selected_index = int(file.readline())
				nodes[curr_node_index][3][selected_index][0] = CHILD_PATH_STATE_SELECTED

		action_sequence = []
		action_sequence_size = int(file.readline())
		for _ in range(action_sequence_size):
			write = float(file.readline())
			move = int(file.readline())
			action_sequence.append([write, move])

		underlying_world_size = int(file.readline())
		underlying_world = []
		for _ in range(underlying_world_size):
			underlying_world.append(float(file.readline()))

	target_result = underlying_world.copy()
	target_result.sort()

	halt_node = pydot.Node(0, label='HALT')
	graph.add_node(halt_node)
	
	for n_index in range(1, len(nodes)):
		if nodes[n_index][0] == NODE_STATE_NONE:
			label = ''
			color = 'black'
		elif nodes[n_index][0] == NODE_STATE_NEW:
			label = 'new'
			color = 'green'
		elif nodes[n_index][0] == NODE_STATE_NEW_CHILD:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'green'
		elif nodes[n_index][0] == NODE_STATE_RANDOM:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'orange'
		elif nodes[n_index][0] == NODE_STATE_ON_PATH:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'green'
		elif nodes[n_index][0] == NODE_STATE_GOOD_EXPLORE:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'blue'
		elif nodes[n_index][0] == NODE_STATE_RANDOM_EXPLORE:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'purple'
		elif nodes[n_index][0] == NODE_STATE_FORCED_EXPLORE:
			label = 'A: ' + str(nodes[n_index][4])
			color = 'grey'

		node = pydot.Node(n_index, label=label, color=color)
		graph.add_node(node)

	for n_index in range(1, len(nodes)):
		for c_index in range(len(nodes[n_index][1])):
			if nodes[n_index][3][c_index][0] == CHILD_PATH_STATE_NONE:
				label = pretty_print_action(nodes[n_index][2][c_index])
				color = 'black'
			elif nodes[n_index][3][c_index][0] == CHILD_PATH_STATE_NEW:
				label = pretty_print_action(nodes[n_index][2][c_index])
				color = 'green'
			elif nodes[n_index][3][c_index][0] == CHILD_PATH_STATE_RANDOM:
				label = pretty_print_action(nodes[n_index][2][c_index])
				color = 'orange'
			elif nodes[n_index][3][c_index][0] == CHILD_PATH_STATE_PROCESSED:
				label = pretty_print_action(nodes[n_index][2][c_index])
				label = label + '\n' + 'S: ' + str(nodes[n_index][3][c_index][1]) \
					+ '\n' + 'M: ' + str(nodes[n_index][3][c_index][2])
				color = 'black'
			else:
				label = pretty_print_action(nodes[n_index][2][c_index])
				label = label + '\n' + 'S: ' + str(nodes[n_index][3][c_index][1]) \
					+ '\n' + 'M: ' + str(nodes[n_index][3][c_index][2])
				color = 'green'

			edge = pydot.Edge(n_index, nodes[n_index][1][c_index], label=label, color=color)
			graph.add_edge(edge)

	graph.write_png('solution.png')

	pointer = 0
	last_pointer = 0
	
	def draw_underlying():
		global ax
		global pointer
		global last_pointer

		img = Image.open('solution.png')
		width, height = img.size
		if width > height:
			scale = 300/width
			margin = (300-int(height*scale))/2
			ax.imshow(img, extent=(25, 325, 305-margin, 5+margin))
		else:
			scale = 300/height
			margin = (300-int(width*scale))/2
			ax.imshow(img, extent=(25+margin, 325-margin, 305, 5))

		squares_width = 100*(underlying_world_size+2)
		squares_scale = 300/squares_width
		square_width = int(100*squares_scale)
		squares_margin = (100-square_width)/2

		l_lim = patches.Rectangle(
			(25, 310+squares_margin),
			square_width,
			square_width,
			linewidth=1,
			edgecolor='r',
			facecolor='r')
		ax.add_patch(l_lim)

		r_lim = patches.Rectangle(
			(25+square_width*(len(underlying_world)+1), 310+squares_margin),
			square_width,
			square_width,
			linewidth=1,
			edgecolor='r',
			facecolor='r')
		ax.add_patch(r_lim)

		for u_index in range(len(underlying_world)):
			rect = patches.Rectangle(
				(25+square_width*(u_index+1), 310+squares_margin),
				square_width,
				square_width,
				linewidth=1,
				edgecolor='black',
				facecolor='none')
			ax.add_patch(rect)
			ax.annotate(
				"{:.1f}".format(underlying_world[u_index]),
				xy=(25+int(square_width*(u_index+1.5)), 310+squares_margin+int(0.5*square_width)),
				fontsize=8,
				horizontalalignment='center',
				verticalalignment='center')

		ax.annotate(
			"",
			xy=(25+int(square_width*(last_pointer+1.5)), 310+squares_margin+square_width),
			xytext=(25+int(square_width*(last_pointer+1.5)), 310+squares_margin+square_width+30),
			arrowprops=dict(arrowstyle="->", color='lightgrey'))
		ax.annotate(
			"",
			xy=(25+int(square_width*(pointer+1.5)), 310+squares_margin+square_width),
			xytext=(25+int(square_width*(pointer+1.5)), 310+squares_margin+square_width+30),
			arrowprops=dict(arrowstyle="->"))

	for a_index in range(len(action_sequence)):
		plt.cla()
		ax.set_xlim([0, 350])
		ax.set_ylim([500, 0])
		ax.axis('off')

		draw_underlying()

		if a_index == 0:
			left_operation = '-'
		else:
			left_operation = pretty_print_action(action_sequence[a_index-1])
		ax.annotate(
			'#'+str(a_index)+': '+left_operation,
			xy=(175, 440),
			fontsize=12,
			color='lightgrey',
			horizontalalignment='center',
			verticalalignment='center')
		right_operation = pretty_print_action(action_sequence[a_index])
		ax.annotate(
			'#'+str(a_index+1)+': '+right_operation,
			xy=(175, 470),
			fontsize=12,
			horizontalalignment='center',
			verticalalignment='center')

		plt.gca().set_aspect('equal')
		fig.canvas.draw()
		fig.canvas.flush_events()
		time.sleep(2)

		last_pointer = pointer
		if pointer >= 0 and pointer < len(underlying_world):
			underlying_world[pointer] += action_sequence[a_index][0]
		if action_sequence[a_index][1] == LEFT:
			if pointer >= 0:
				pointer -= 1
		elif action_sequence[a_index][1] == RIGHT:
			if pointer < len(underlying_world):
				pointer += 1

	final_result = True
	for u_index in range(len(underlying_world)):
		if abs(underlying_world[u_index]-target_result[u_index]) > 0.1:
			final_result = False
			break

	plt.cla()
	ax.set_xlim([0, 350])
	ax.set_ylim([500, 0])
	ax.axis('off')

	draw_underlying()

	if len(action_sequence) == 0:
		left_operation = '-'
	else:
		left_operation = pretty_print_action(action_sequence[-1])
	ax.annotate(
		'#'+str(len(action_sequence))+': '+left_operation,
		xy=(175, 440),
		fontsize=12,
		color='lightgrey',
		horizontalalignment='center',
		verticalalignment='center')
	if final_result:
		ax.annotate(
			'#'+str(len(action_sequence)+1)+': HALT (SUCCESS)',
			xy=(175, 470),
			fontsize=12,
			color='green',
			horizontalalignment='center',
			verticalalignment='center')

		ax.axis('on')
		ax.tick_params(
			which='both',
			bottom=False,
			labelbottom=False,
			left=False,
			labelleft=False)
		for spine in ax.spines.values():
			spine.set_edgecolor('green')
	else:
		ax.annotate(
			'#'+str(len(action_sequence)+1)+': HALT (FAIL)',
			xy=(175, 470),
			fontsize=12,
			color='red',
			horizontalalignment='center',
			verticalalignment='center')

		ax.axis('on')
		ax.tick_params(
			which='both',
			bottom=False,
			labelbottom=False,
			left=False,
			labelleft=False)
		for spine in ax.spines.values():
			spine.set_edgecolor('red')

	plt.gca().set_aspect('equal')
	fig.canvas.draw()
	fig.canvas.flush_events()
	time.sleep(8.0)
