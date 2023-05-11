import pydot

ACTION_START = -1
ACTION_LEFT = 0
ACTION_STAY = 1
ACTION_RIGHT = 2

NODE_TYPE_ACTION = 0;
NODE_TYPE_INNER_SCOPE = 1;
NODE_TYPE_BRANCH = 2;
NODE_TYPE_FOLD_SCORE = 3;
NODE_TYPE_FOLD_SEQUENCE = 4;
NODE_TYPE_LOOP_FOLD = 5;
NODE_TYPE_PASS_THROUGH = 6;

scopes = []

file = open('../display.txt')

num_scopes = int(file.readline())

for s_index in range(num_scopes):
	is_loop = int(file.readline())

	nodes = []
	num_nodes = int(file.readline())
	for n_index in range(num_nodes):
		node_type = int(file.readline())
		if node_type == NODE_TYPE_ACTION:
			action = int(file.readline())

			next_node_id = int(file.readline())

			average_score = float(file.readline())
			score_variance = float(file.readline())
			average_misguess = float(file.readline())
			misguess_variance = float(file.readline())

			average_impact = float(file.readline())
			average_sum_impact = float(file.readline())

			nodes.append([[node_type,
						   action,
						   next_node_id,
						   average_score,
						   score_variance,
						   average_misguess,
						   misguess_variance,
						   average_impact,
						   average_sum_impact], -1])
		elif node_type == NODE_TYPE_INNER_SCOPE:
			inner_scope_id = int(file.readline())

			next_node_id = int(file.readline())

			average_score = float(file.readline())
			score_variance = float(file.readline())
			average_misguess = float(file.readline())
			misguess_variance = float(file.readline())

			average_impact = float(file.readline())
			average_sum_impact = float(file.readline())

			nodes.append([[node_type,
						   inner_scope_id,
						   next_node_id,
						   average_score,
						   score_variance,
						   average_misguess,
						   misguess_variance,
						   average_impact,
						   average_sum_impact], -1])
		elif node_type == NODE_TYPE_BRANCH:
			original_next_node_id = int(file.readline())

			branch_scope_id = int(file.readline())
			branch_next_node_id = int(file.readline())

			nodes.append([[node_type,
						   original_next_node_id,
						   branch_scope_id,
						   branch_next_node_id], -1])
		elif node_type == NODE_TYPE_FOLD_SCORE:
			existing_next_node_id = int(file.readline())

			fold_scope_id = int(file.readline())
			fold_next_node_id = int(file.readline())

			nodes.append([[node_type,
						   existing_next_node_id,
						   fold_scope_id,
						   fold_next_node_id], -1])
		elif node_type == NODE_TYPE_FOLD_SEQUENCE:
			next_node_id = int(file.readline())

			nodes.append([[node_type,
						   next_node_id], -1])
		elif node_type == NODE_TYPE_LOOP_FOLD:
			next_node_id = int(file.readline())

			nodes.append([[node_type,
						   next_node_id], -1])
		else:
			# node_type == NODE_TYPE_LOOP_FOLD
			next_node_id = int(file.readline())

			nodes.append([[node_type,
						   next_node_id], -1])

	scopes.append([is_loop,
				   nodes,
				   -1,
				   -1])

file.close()

print(scopes)

def pretty_print_action(action):
	result = ''
	if action == -1:
		result = 'START'
	elif action == 0:
		result = 'LEFT'
	elif action == 1:
		result = 'RIGHT'
	elif action == 2:
		result = 'SWAP'
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

global_node_index = 0

for s_index in range(len(scopes)):
	start_node_index = global_node_index
	global_node_index += 1

	if scopes[s_index][0] == 1:
		start_node = pydot.Node(start_node_index, label='LOOP S'+str(s_index)+' START')
	else:
		start_node = pydot.Node(start_node_index, label='S'+str(s_index)+' START')
	graph.add_node(start_node)
	scopes[s_index][2] = start_node_index

	for n_index in range(len(scopes[s_index][1])):
		node_index = global_node_index
		global_node_index += 1
		if scopes[s_index][1][n_index][0][0] == NODE_TYPE_ACTION:
			node = pydot.Node(node_index, label=pretty_print_action(scopes[s_index][1][n_index][0][1]))
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_INNER_SCOPE:
			node = pydot.Node(node_index, label='S'+str(scopes[s_index][1][n_index][0][1]))
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_BRANCH:
			node = pydot.Node(node_index, label='B')
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_FOLD_SCORE:
			node = pydot.Node(node_index, label='FOLD START')
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_FOLD_SEQUENCE:
			node = pydot.Node(node_index, label='FOLD SEQUENCE')
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_LOOP_FOLD:
			node = pydot.Node(node_index, label='LOOP FOLD')
		else:
			# scopes[s_index][1][n_index][0][0] == NODE_TYPE_PASS_THROUGH
			node = pydot.Node(node_index, label='PASS')
		graph.add_node(node)
		scopes[s_index][1][n_index][1] = node_index

	end_node_index = global_node_index
	global_node_index += 1

	end_node = pydot.Node(end_node_index, label='S'+str(s_index)+' END')
	graph.add_node(end_node)
	scopes[s_index][3] = end_node_index

for s_index in range(len(scopes)):
	start_edge = pydot.Edge(scopes[s_index][2], scopes[s_index][1][0][1])
	graph.add_edge(start_edge)

	for n_index in range(len(scopes[s_index][1])):
		if scopes[s_index][1][n_index][0][0] == NODE_TYPE_ACTION:
			if scopes[s_index][1][n_index][0][2] == -1:
				edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][2]][1])
			graph.add_edge(edge)
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_INNER_SCOPE:
			if s_index < scopes[s_index][1][n_index][0][1]:
				input_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[scopes[s_index][1][n_index][0][1]][2])
				graph.add_edge(input_edge)

				if scopes[s_index][1][n_index][0][2] == -1:
					output_edge = pydot.Edge(scopes[scopes[s_index][1][n_index][0][1]][3],
						scopes[s_index][3])
				else:
					output_edge = pydot.Edge(scopes[scopes[s_index][1][n_index][0][1]][3],
						scopes[s_index][1][scopes[s_index][1][n_index][0][2]][1])
				graph.add_edge(output_edge)
			else:
				if scopes[s_index][1][n_index][0][2] == -1:
					edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
				else:
					edge = pydot.Edge(scopes[s_index][1][n_index][1],
						scopes[s_index][1][scopes[s_index][1][n_index][0][2]][1])
				graph.add_edge(edge)
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_BRANCH:
			if scopes[s_index][1][n_index][0][1] == -1:
				original_edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				original_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][1]][1])
			graph.add_edge(original_edge)

			if scopes[s_index][1][n_index][0][3] == -1:
				branch_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[scopes[s_index][1][n_index][0][2]][3])
			else:
				branch_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[scopes[s_index][1][n_index][0][2]][1][scopes[s_index][1][n_index][0][3]][1])
			graph.add_edge(branch_edge)
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_FOLD_SCORE:
			if scopes[s_index][1][n_index][0][1] == -1:
				existing_edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				existing_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][1]][1])
			graph.add_edge(existing_edge)

			if scopes[s_index][1][n_index][0][3] == -1:
				fold_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[scopes[s_index][1][n_index][0][2]][3])
			else:
				fold_edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[scopes[s_index][1][n_index][0][2]][1][scopes[s_index][1][n_index][0][3]][1])
			graph.add_edge(fold_edge)
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_FOLD_SEQUENCE:
			if scopes[s_index][1][n_index][0][1] == -1:
				edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][1]][1])
			graph.add_edge(edge)
		elif scopes[s_index][1][n_index][0][0] == NODE_TYPE_LOOP_FOLD:
			if scopes[s_index][1][n_index][0][1] == -1:
				edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][1]][1])
			graph.add_edge(edge)
		else:
			# scopes[s_index][1][n_index][0][0] == NODE_TYPE_PASS_THROUGH
			if scopes[s_index][1][n_index][0][1] == -1:
				edge = pydot.Edge(scopes[s_index][1][n_index][1], scopes[s_index][3])
			else:
				edge = pydot.Edge(scopes[s_index][1][n_index][1],
					scopes[s_index][1][scopes[s_index][1][n_index][0][1]][1])
			graph.add_edge(edge)

graph.write_png('solution.png')
