import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
import pydot
import time
from PIL import Image, ImageOps

NODE_TYPE_ACTION = 0;
NODE_TYPE_IF_START = 1;
NODE_TYPE_IF_END = 2;
NODE_TYPE_LOOP_START = 3;
NODE_TYPE_LOOP_END = 4;

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
	with open('../display.txt') as file:
		num_nodes = int(file.readline())
		for n_index in range(num_nodes):
			node_is_on = int(file.readline())
			if node_is_on == 1:
				if n_index == 1:
					nodes.append([True, NODE_TYPE_LOOP_END, -1, -1])
					continue

				node_type = int(file.readline())
				if node_type == NODE_TYPE_ACTION:
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

	for n_index in range(0, len(nodes)):
		if nodes[n_index][0]:
			if n_index == 0:
				label = 'start'
			elif n_index == 1:
				label = 'halt'
			elif nodes[n_index][1] == NODE_TYPE_ACTION:
				label = pretty_print_action(nodes[n_index][2])
			elif nodes[n_index][1] == NODE_TYPE_IF_START:
				label = ''
			elif nodes[n_index][1] == NODE_TYPE_IF_END:
				label = ''
			elif nodes[n_index][1] == NODE_TYPE_LOOP_START:
				label = ''
			elif nodes[n_index][1] == NODE_TYPE_LOOP_END:
				label = ''
			node = pydot.Node(n_index, label=label)
			graph.add_node(node)

	for n_index in range(0, len(nodes)):
		if nodes[n_index][0]:
			if n_index == 1:
				pass
			elif nodes[n_index][1] == NODE_TYPE_ACTION:
				edge = pydot.Edge(n_index, nodes[n_index][3])
				graph.add_edge(edge)
			elif nodes[n_index][1] == NODE_TYPE_IF_START:
				for c_index in range(len(nodes[n_index][2])):
					if nodes[n_index][2][c_index][0]:
						edge = pydot.Edge(n_index, nodes[n_index][2][c_index][1])
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

	graph.write_png('solution.png')

	plt.cla()
	ax.axis('off')

	img = Image.open('solution.png')
	ax.imshow(img)

	plt.gca().set_aspect('equal')
	fig.canvas.draw()
	fig.canvas.flush_events()
	time.sleep(10)
