import pydot

ACTION_UP = 0
ACTION_RIGHT = 1
ACTION_DOWN = 2
ACTION_LEFT = 3

fixed_points = {}

file = open('../saves/display.txt')

num_fixed_points = int(file.readline())
for p_index in range(num_fixed_points):
	fixed_point_id = int(file.readline())

	val_average = float(file.readline())

	transitions = {}
	num_transitions = int(file.readline())
	for t_index in range(num_transitions):
		next_fixed_point_id = int(file.readline())

		moves = []
		num_moves = int(file.readline())
		for m_index in range(num_moves):
			moves.append(int(file.readline()))

		transitions[next_fixed_point_id] = moves

	fixed_points[fixed_point_id] = [val_average,
									transitions]

file.close()

def pretty_print_action(action):
	result = ''
	if action == 0:
		result = 'UP'
	elif action == 1:
		result = 'RIGHT'
	elif action == 2:
		result = 'DOWN'
	elif action == 3:
		result = 'LEFT'
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

global_node_index = 0

node_mappings = {}
for fixed_point_id in fixed_points:
	node_index = global_node_index
	global_node_index += 1
	graph.add_node(pydot.Node(node_index, label=str(fixed_points[fixed_point_id][0])))
	node_mappings[fixed_point_id] = node_index
for fixed_point_id in fixed_points:
	for next_fixed_point_id in fixed_points[fixed_point_id][1]:
		print('next_fixed_point_id: ' + str(next_fixed_point_id))

		moves = fixed_points[fixed_point_id][1][next_fixed_point_id]

		prev_id = node_mappings[fixed_point_id]
		for m_index in range(len(moves)-1):
			node_index = global_node_index
			global_node_index += 1
			graph.add_node(pydot.Node(node_index, label=''))

			graph.add_edge(pydot.Edge(prev_id, node_index, label=pretty_print_action(moves[m_index])))

			prev_id = node_index

		graph.add_edge(pydot.Edge(prev_id, node_mappings[next_fixed_point_id], label=pretty_print_action(moves[-1])))

graph.write_png('solution.png')
