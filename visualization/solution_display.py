import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
import pydot
import random

def pretty_print_label(action):
	if action[1] == -1:
		return ''

	result = "{:.2f}".format(action[0])
	result += "\n"
	if action[1] == 0:
		result += 'LEFT'
	elif action[1] == 1:
		result += 'STAY'
	else:
		result += 'RIGHT'
	return result

while True:
	graph = pydot.Dot(graph_type='digraph', strict=True)

	nodes = []
	with open('../display.txt') as file:
		num_nodes = int(file.readline())
		for n_index in range(num_nodes):
			is_explore_starting_point = False
			is_on_path = False
			num_children = int(file.readline())
			children_indexes = []
			children_actions = []
			paths_explored = []
			for c_index in range(num_children):
				child_index = int(file.readline())
				children_indexes.append(child_index)
				
				write = float(file.readline())
				move = int(file.readline())
				children_actions.append([write, move])

				paths_explored.append(False)

			explore_state = file.readline().strip()

			nodes.append([is_explore_starting_point,
						  is_on_path,
						  children_indexes,
						  children_actions,
						  paths_explored,
						  explore_state])

		underlying_world_size = int(file.readline())
		underlying_world = []
		for _ in range(underlying_world_size):
			underlying_world.append(float(file.readline()))

		chosen_paths_size = int(file.readline())
		for _ in range(chosen_paths_size):
			chosen_path = file.readline().strip().split(',')
			start = int(chosen_path[0])
			end = int(chosen_path[1])
			nodes[start][1] = True
			nodes[end][1] = True
			nodes[start][4][end] = True

		explore_status = file.readline().strip()

		candidate = []
		if explore_status != 'no_explore':
			explore_node = int(file.readline())
			nodes[explore_node][0] = True

			candidate_size = int(file.readline())
			for _ in range(candidate_size):
				write = float(file.readline())
				move = int(file.readline())
				candidate.append([write, move])

	halt_node = pydot.Node(0, label='HALT')
	graph.add_node(halt_node)
	for n_index in range(1, len(nodes)):
		label = str(nodes[n_index][5])
		color = 'black'
		if nodes[n_index][0]:
			color = 'red'
		elif nodes[n_index][1]:
			color = 'blue'
		node = pydot.Node(n_index, label=label, color=color)
		graph.add_node(node)

	for n_index in range(1, len(nodes)):
		for c_index in range(len(nodes[n_index][2])):
			label = pretty_print_label(nodes[n_index][3][c_index])
			edge = pydot.Edge(n_index, nodes[n_index][2][c_index], label=label)
			graph.add_edge(edge)

	mplstyle.use('fast')
	plt.ion()
	plt.gca().set_aspect('equal')

	graph.write_png('solution.png')

	img = mpimg.imread('solution.png')
	plt.imshow(img)
	plt.axis('off')

	plt.text(0, 0, explore_status)
	if explore_status != 'no_explore':
		plt.text(0, 10, str(candidate))

	plt.show()
	plt.pause(2.0)
	plt.clf()
