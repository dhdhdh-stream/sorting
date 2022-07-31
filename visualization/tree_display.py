import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
import pydot
import random

while True:
	graph = pydot.Dot(graph_type='digraph', strict=True)
	curr_index = 0

	halt_graph_index = curr_index
	curr_index += 1
	halt_node = pydot.Node(halt_graph_index, label='HALT')
	graph.add_node(halt_node)

	class TreeNode:
		def __init__(self, file):
			self.children = []
			self.children_actions = []

			self.has_halt = file.readline().strip()

			self.count = int(file.readline())
			self.score = float(file.readline())
			self.information = float(file.readline())

			num_children = int(file.readline())
			for _ in range(num_children):
				write = float(file.readline())
				move = int(file.readline())

				self.children_actions.append([write, move])
				child = TreeNode(file)
				self.children.append(child)

		def add_to_graph(self, label):
			global graph
			global curr_index

			curr_graph_index = curr_index
			curr_index += 1
			node = pydot.Node(curr_graph_index, label=label)
			graph.add_node(node)

			if self.has_halt == 'true':
				global halt_graph_index
				edge = pydot.Edge(curr_graph_index, halt_graph_index)
				graph.add_edge(edge)

			for c_index in range(len(self.children)):
				child_graph_index = self.children[c_index].add_to_graph(str(self.children_actions[c_index]))
				edge = pydot.Edge(curr_graph_index, child_graph_index)
				graph.add_edge(edge)

			return curr_graph_index

	with open('../tree_display.txt') as file:
		root = TreeNode(file)
		root.add_to_graph('root')

	mplstyle.use('fast')
	plt.ion()
	plt.gca().set_aspect('equal')

	graph.write_png('tree.png')

	img = mpimg.imread('tree.png')
	plt.imshow(img)
	plt.axis('off')
	plt.show()
	plt.pause(2.0)
	plt.clf()
