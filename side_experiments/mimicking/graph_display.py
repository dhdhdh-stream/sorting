import pydot

ACTION_NOOP = -1
ACTION_UP = 0
ACTION_RIGHT = 1
ACTION_DOWN = 2
ACTION_LEFT = 3
ACTION_CLICK = 4
ACTION_FLAG = 5
ACTION_DOUBLECLICK = 6

nodes = []

file = open('display.txt')

num_nodes = int(file.readline())

for _ in range(num_nodes):
	action = int(file.readline())

	num_next_nodes = int(file.readline())
	next_nodes = []
	for n_index in range(num_next_nodes):
		next_nodes.append(int(file.readline()))

	nodes.append([action, next_nodes])

file.close()

def pretty_print_action(action):
	result = ''
	if action == -1:
		result = 'NOOP'
	elif action == 0:
		result = 'UP'
	elif action == 1:
		result = 'RIGHT'
	elif action == 2:
		result = 'DOWN'
	elif action == 3:
		result = 'LEFT'
	elif action == 4:
		result = 'CLICK'
	elif action == 5:
		result = 'FLAG'
	elif action == 6:
		result = 'DOUBLECLICK'
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

for node_id in range(num_nodes):
	graph.add_node(pydot.Node(node_id, label=pretty_print_action(nodes[node_id][0])))

for node_id in range(num_nodes):
	for next_node_id in nodes[node_id][1]:
		graph.add_edge(pydot.Edge(node_id, next_node_id))

graph.write_png('graph.png')
