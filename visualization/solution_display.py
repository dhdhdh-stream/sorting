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
	with open('../solution_display.txt') as file:
		num_nodes = int(file.readline())
		for n_index in range(num_nodes):
			count = int(file.readline())
			information = float(file.readline())
			
			num_children = int(file.readline())
			children_indexes = []
			children_actions = []
			for c_index in range(num_children):
				child_index = int(file.readline())
				children_indexes.append(child_index)
				
				write = float(file.readline())
				move = int(file.readline())
				children_actions.append([write, move])

				network_name = file.readline()

			nodes.append([count, information, children_indexes, children_actions])

	halt_node = pydot.Node(0, label='HALT')
	graph.add_node(halt_node)
	for n_index in range(1, len(nodes)):
		label = str(nodes[n_index][0]) + '\n' + str(nodes[n_index][1])
		node = pydot.Node(n_index, label=label)
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
	plt.show()
	plt.pause(2.0)
	plt.clf()
