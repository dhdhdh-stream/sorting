import pydot

states = []

file = open('display.txt')

num_states = int(file.readline())

for s_index in range(num_states):
	state_id = int(file.readline())

	val_average = float(file.readline())

	obs_transitions = []

	num_obs_transitions = int(file.readline())
	for t_index in range(num_obs_transitions):
		obs_index = int(file.readline())
		next_state = int(file.readline())

		obs_transitions.append([obs_index, next_state])

	action_transitions = []

	num_action_transitions = int(file.readline())
	for t_index in range(num_action_transitions):
		action_id = int(file.readline())
		next_state = int(file.readline())

		action_transitions.append([action_id, next_state])

	default_transition = int(file.readline())

	states.append([state_id,
				   val_average,
				   obs_transitions,
				   action_transitions,
				   default_transition])

file.close()

print(states)

graph = pydot.Dot(graph_type='digraph', strict=True)

for state in states:
	graph.add_node(pydot.Node(state[0], label=str(state[1])))

for state in states:
	for obs_transition in state[2]:
		graph.add_edge(pydot.Edge(state[0], obs_transition[1], label='O ' + str(obs_transition[0])))
	for action_transition in state[3]:
		graph.add_edge(pydot.Edge(state[0], action_transition[1], label='A ' + str(action_transition[0])))
	graph.add_edge(pydot.Edge(state[0], state[4], label='D'))

graph.write_png('world_model.png');
