import matplotlib.patches as patches
import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
import pydot
import time
from PIL import Image, ImageOps

LEFT = 0
STAY = 1
RIGHT = 2

NODE_TYPE_START = 0;
NODE_TYPE_END = 1;
NODE_TYPE_ACTION = 2;
NODE_TYPE_IF_START = 3;
NODE_TYPE_IF_END = 4;
NODE_TYPE_LOOP_START = 5;
NODE_TYPE_LOOP_END = 6;

def pretty_print_action(action):
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
	run_data = []
	underlying_world = []
	with open('../display.txt') as file:
		num_nodes = int(file.readline())
		for n_index in range(num_nodes):
			node_is_on = int(file.readline())
			if node_is_on == 1:
				node_type = int(file.readline())
				if node_type == NODE_TYPE_START:
					next_index = int(file.readline())
					nodes.append([True, node_type, next_index])
				elif node_type == NODE_TYPE_END:
					nodes.append([True, node_type])
				elif node_type == NODE_TYPE_ACTION:
					write = float(file.readline())
					move = int(file.readline())
					next_index = int(file.readline())
					nodes.append([True, node_type, [write, move], next_index])
				elif node_type == NODE_TYPE_IF_START:
					num_children = int(file.readline())
					children_indexes = []
					for c_index in range(num_children):
						child_node_is_on = int(file.readline())
						if child_node_is_on == 1:
							children_indexes.append([True, int(file.readline())])
						else:
							children_indexes.append([False])
					nodes.append([True, node_type, children_indexes])
				elif node_type == NODE_TYPE_IF_END:
					next_index = int(file.readline())
					nodes.append([True, node_type, next_index])
				elif node_type == NODE_TYPE_LOOP_START:
					next_index = int(file.readline())
					nodes.append([True, node_type, next_index])
				elif node_type == NODE_TYPE_LOOP_END:
					start_index = int(file.readline())
					next_index = int(file.readline())
					nodes.append([True, node_type, start_index, next_index])
			else:
				nodes.append([False])

		while True:
			node_index = int(file.readline())
			if node_index == -1:
				write = float(file.readline())
				move = int(file.readline())
				run_data.append([-1, [write, move]])
			elif nodes[node_index][1] == NODE_TYPE_START:
				run_data.append([node_index])
			elif nodes[node_index][1] == NODE_TYPE_END:
				run_data.append([node_index])
				break
			elif nodes[node_index][1] == NODE_TYPE_ACTION:
				write = float(file.readline())
				move = int(file.readline())
				run_data.append([node_index, [write, move]])
			elif nodes[node_index][1] == NODE_TYPE_IF_START:
				num_scores = int(file.readline())
				if num_scores == -1:
					best_index = int(file.readline())
					best_score = float(file.readline())
					run_data.append([node_index, -1, best_index, best_score])
				else:
					child_scores = []
					for c_index in range(num_scores):
						is_on = file.readline().strip()
						if is_on == 'on':
							child_score = float(file.readline())
							child_scores.append([True, child_score])
						else:
							child_scores.append([False])
					best_index = int(file.readline())
					run_data.append([node_index, num_scores, child_scores, best_index])
			else:
				run_data.append([node_index])

		underlying_world_size = int(file.readline())
		for _ in range(underlying_world_size):
			underlying_world.append(float(file.readline()))

		is_explore = file.readline().strip()
		if is_explore == 'explore':
			explore_node_index = int(file.readline())
			is_path = file.readline().strip()
			if is_path == 'path':
				target_node = int(file.readline())
				explore_data = [True, explore_node_index, True, target_node]
			else:
				explore_data = [True, explore_node_index, False]
		else:
			explore_data = [False]

	target_result = underlying_world.copy()
	target_result.sort()

	curr_action_index = -1

	pointer = 0
	last_pointer = 0

	def draw_underlying(curr_index, explore_label):
		global ax
		global pointer
		global last_pointer

		for n_index in range(0, len(nodes)):
			if nodes[n_index][0]:
				if nodes[n_index][1] == NODE_TYPE_START:
					label = 'start'
				elif nodes[n_index][1] == NODE_TYPE_END:
					label = 'halt'
				elif nodes[n_index][1] == NODE_TYPE_ACTION:
					label = str(n_index) + ' ' + pretty_print_action(nodes[n_index][2])
				elif nodes[n_index][1] == NODE_TYPE_IF_START:
					label = str(n_index)
				elif nodes[n_index][1] == NODE_TYPE_IF_END:
					label = str(n_index)
				elif nodes[n_index][1] == NODE_TYPE_LOOP_START:
					label = str(n_index)
				elif nodes[n_index][1] == NODE_TYPE_LOOP_END:
					label = str(n_index)
				
				if explore_data[0] == True and explore_data[1] == n_index:
					if explore_label != '':
						label += '\n' + explore_label
					node = pydot.Node(n_index, label=label, color='blue')
				elif run_data[curr_index][0] == n_index:
					node = pydot.Node(n_index, label=label, color='green')
				else:
					node = pydot.Node(n_index, label=label, color='black')
				graph.add_node(node)

		for n_index in range(0, len(nodes)):
			if nodes[n_index][0]:
				if nodes[n_index][1] == NODE_TYPE_START:
					edge = pydot.Edge(n_index, nodes[n_index][2])
					graph.add_edge(edge)
				elif nodes[n_index][1] == NODE_TYPE_END:
					pass
				elif nodes[n_index][1] == NODE_TYPE_ACTION:
					edge = pydot.Edge(n_index, nodes[n_index][3])
					graph.add_edge(edge)
				elif nodes[n_index][1] == NODE_TYPE_IF_START:
					if run_data[curr_index][0] == n_index:
						if run_data[curr_index][1] != -1:
							for c_index in range(len(nodes[n_index][2])):
								if nodes[n_index][2][c_index][0]:
									if c_index == run_data[curr_index][3]:
										label = run_data[curr_index][2][c_index][1]
										edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1], label=label, color='green')
										graph.add_edge(edge)
									else:
										label = run_data[curr_index][2][c_index][1]
										edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1], label=label, color='black')
										graph.add_edge(edge)
						else:
							for c_index in range(len(nodes[n_index][2])):
								if nodes[n_index][2][c_index][0]:
									if c_index == run_data[curr_index][2]:
										edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1], label='', color='orange')
										graph.add_edge(edge)
									else:
										edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1], label='', color='black')
										graph.add_edge(edge)
					else:
						for c_index in range(len(nodes[n_index][2])):
							if nodes[n_index][2][c_index][0]:
								edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1], label='', color='black')
								graph.add_edge(edge)
				elif nodes[n_index][1] == NODE_TYPE_IF_END:
					edge = pydot.Edge(n_index, nodes[n_index][2])
					graph.add_edge(edge)
				elif nodes[n_index][1] == NODE_TYPE_LOOP_START:
					edge = pydot.Edge(n_index, nodes[n_index][2])
					graph.add_edge(edge)
				elif nodes[n_index][1] == NODE_TYPE_LOOP_END:
					edge = pydot.Edge(n_index, nodes[n_index][2])
					graph.add_edge(edge)
					edge = pydot.Edge(n_index, nodes[n_index][3])
					graph.add_edge(edge)

				if explore_data[0] == True \
						and explore_data[1] == n_index \
						and explore_data[2] == True:
					edge = pydot.Edge(n_index, explore_data[3], color='blue', style='dashed')
					graph.add_edge(edge)

		graph.write_png('solution.png')

		img = Image.open('solution.png')
		width, height = img.size
		if width > height:
			scale = 500/width
			margin = (500-int(height*scale))/2
			ax.imshow(img, extent=(25, 525, 505-margin, 5+margin))
		else:
			scale = 500/height
			margin = (500-int(width*scale))/2
			ax.imshow(img, extent=(25+margin, 525-margin, 505, 5))

		squares_width = 100*(underlying_world_size+2)
		squares_scale = 300/squares_width
		square_width = int(100*squares_scale)
		squares_margin = (100-square_width)/2

		l_lim = patches.Rectangle(
			(125, 510+squares_margin),
			square_width,
			square_width,
			linewidth=1,
			edgecolor='r',
			facecolor='r')
		ax.add_patch(l_lim)

		r_lim = patches.Rectangle(
			(125+square_width*(len(underlying_world)+1), 510+squares_margin),
			square_width,
			square_width,
			linewidth=1,
			edgecolor='r',
			facecolor='r')
		ax.add_patch(r_lim)

		for u_index in range(len(underlying_world)):
			rect = patches.Rectangle(
				(125+square_width*(u_index+1), 510+squares_margin),
				square_width,
				square_width,
				linewidth=1,
				edgecolor='black',
				facecolor='none')
			ax.add_patch(rect)
			ax.annotate(
				"{:.1f}".format(underlying_world[u_index]),
				xy=(125+int(square_width*(u_index+1.5)), 510+squares_margin+int(0.5*square_width)),
				fontsize=9,
				horizontalalignment='center',
				verticalalignment='center')

		ax.annotate(
			"",
			xy=(125+int(square_width*(last_pointer+1.5)), 510+squares_margin+square_width),
			xytext=(125+int(square_width*(last_pointer+1.5)), 510+squares_margin+square_width+30),
			arrowprops=dict(arrowstyle="->", color='lightgrey'))
		ax.annotate(
			"",
			xy=(125+int(square_width*(pointer+1.5)), 510+squares_margin+square_width),
			xytext=(125+int(square_width*(pointer+1.5)), 510+squares_margin+square_width+30),
			arrowprops=dict(arrowstyle="->"))

	for curr_index in range(len(run_data)):
		plt.cla()
		ax.set_xlim([0, 550])
		ax.set_ylim([620, 0])
		ax.axis('off')

		if run_data[curr_index][0] == -1 \
				or nodes[run_data[curr_index][0]][1] == NODE_TYPE_ACTION:
			last_pointer = pointer
			if pointer >= 0 and pointer < len(underlying_world):
				underlying_world[pointer] += run_data[curr_index][1][0]
			if run_data[curr_index][1][1] == LEFT:
				if pointer >= 0:
					pointer -= 1
			elif run_data[curr_index][1][1] == RIGHT:
				if pointer < len(underlying_world):
					pointer += 1

		if run_data[curr_index][0] == -1:
			explore_label = pretty_print_action(run_data[curr_index][1])
		else:
			explore_label = ''

		draw_underlying(curr_index, explore_label)

		ax.annotate(
			'#'+str(curr_index),
			xy=(10, 10),
			fontsize=12,
			horizontalalignment='left',
			verticalalignment='top')

		plt.gca().set_aspect('equal')
		plt.tight_layout()
		fig.canvas.draw()
		fig.canvas.flush_events()
		time.sleep(2)

	final_result = True
	for u_index in range(len(underlying_world)):
		if abs(underlying_world[u_index]-target_result[u_index]) > 0.1:
			final_result = False
			break

	plt.cla()
	ax.set_xlim([0, 550])
	ax.set_ylim([620, 0])
	ax.axis('off')

	draw_underlying(len(run_data)-1, '')

	if final_result:
		ax.annotate(
			'SUCCESS',
			xy=(10, 10),
			fontsize=12,
			horizontalalignment='left',
			verticalalignment='top',
			color='green')

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
			'FAIL',
			xy=(10, 10),
			fontsize=12,
			horizontalalignment='left',
			verticalalignment='top',
			color='red')

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
	plt.tight_layout()
	fig.canvas.draw()
	fig.canvas.flush_events()
	time.sleep(8)
