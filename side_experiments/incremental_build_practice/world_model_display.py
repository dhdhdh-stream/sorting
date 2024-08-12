import pydot

NUM_ACTIONS = 4
ACTION_UP = 0
ACTION_RIGHT = 1
ACTION_DOWN = 2
ACTION_LEFT = 3

file = open('display.txt')

num_states = int(file.readline())

states = []
for s_index in range(num_states):
	average_val = float(file.readline())

	transitions = []
	for a_index in range(NUM_ACTIONS):
		action_transitions = []
		for is_index in range(num_states):
			action_transitions.append(float(file.readline()))
		transitions.append(action_transitions)

	states.append([average_val, transitions])

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

for s_index in range(num_states):
	graph.add_node(pydot.Node(global_node_index, label=str(s_index) + '\n' + str(states[s_index][0])))
	global_node_index += 1

for s_index in range(num_states):
	for a_index in range(NUM_ACTIONS):
		for is_index in range(num_states):
			if states[s_index][1][a_index][is_index] > 0.5:
				graph.add_node(pydot.Node(global_node_index, label=pretty_print_action(a_index) + '\n' + str(states[s_index][1][a_index][is_index])))
				graph.add_edge(pydot.Edge(s_index, global_node_index))
				graph.add_edge(pydot.Edge(global_node_index, is_index))
				global_node_index += 1
			elif states[s_index][1][a_index][is_index] > 0.2:
				graph.add_node(pydot.Node(global_node_index, label=pretty_print_action(a_index) + '\n' + str(states[s_index][1][a_index][is_index])))
				graph.add_edge(pydot.Edge(s_index, global_node_index, style="dotted"))
				graph.add_edge(pydot.Edge(global_node_index, is_index, style="dotted"))
				global_node_index += 1

graph.write_png('world_model.png')
