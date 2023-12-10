import pydot

states = []

file = open('display.txt')

num_states = int(file.readline())

for s_index in range(num_states):
	state_id = int(file.readline())

	average_val = float(file.readline())

	transitions = []

	num_transitions = int(file.readline())
	for t_index in range(num_transitions):
		action = int(file.readline())
		next_state = int(file.readline())

		transitions.append([action, next_state])

	states.append([state_id, average_val, transitions])

file.close()

print(states)

graph = pydot.Dot(graph_type='digraph', strict=True)

for state in states:
	graph.add_node(pydot.Node(state[0], label=str(state[1])))

for state in states:
	for transition in state[2]:
		graph.add_edge(pydot.Edge(state[0], transition[1], label=str(transition[0])))

graph.write_png('world_model.png');
