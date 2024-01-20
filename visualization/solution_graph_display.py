import pydot

# ACTION_NOOP = -1
# ACTION_LEFT = 0
# ACTION_STAY = 1
# ACTION_RIGHT = 2
ACTION_NOOP = -1
ACTION_UP = 0
ACTION_RIGHT = 1
ACTION_DOWN = 2
ACTION_LEFT = 3
ACTION_CLICK = 4
ACTION_FLAG = 5;

NODE_TYPE_ACTION = 0
NODE_TYPE_SCOPE = 1
NODE_TYPE_BRANCH = 2
NODE_TYPE_EXIT = 3

scopes = {}

file = open('../display.txt')

num_scopes = int(file.readline())

for s_index in range(num_scopes):
	scope_id = int(file.readline())

	nodes = {}
	num_nodes = int(file.readline())
	for n_index in range(num_nodes):
		node_id = int(file.readline())
		node_type = int(file.readline())
		if node_type == NODE_TYPE_ACTION:
			action = int(file.readline())

			next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  action,
							  next_node_id]
		elif node_type == NODE_TYPE_SCOPE:
			inner_scope_id = int(file.readline())

			next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  inner_scope_id,
							  next_node_id]
		elif node_type == NODE_TYPE_BRANCH:
			scope_context = int(file.readline())

			is_pass_through = bool(file.readline())

			original_next_node_id = int(file.readline())
			branch_next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  scope_context,
							  original_next_node_id,
							  branch_next_node_id]
		else:
			# node_type == NODE_TYPE_EXIT
			exit_depth = int(file.readline())

			exit_is_exit = bool(file.readline())

			exit_node_parent_id = int(file.readline())
			exit_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  exit_depth,
							  exit_node_parent_id,
							  exit_node_id]

	print(nodes)

	scopes[scope_id] = nodes

file.close()

print(scopes)

# def pretty_print_action(action):
# 	result = ''
# 	if action == -1:
# 		result = 'NOOP'
# 	elif action == 0:
# 		result = 'LEFT'
# 	elif action == 1:
# 		result = 'RIGHT'
# 	elif action == 2:
# 		result = 'SWAP'
# 	return result
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
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

global_node_index = 0

for scope_id in scopes:
	node_mappings = {}
	for key in scopes[scope_id]:
		node_index = global_node_index
		global_node_index += 1
		if scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + pretty_print_action(scopes[scope_id][key][1])))
		elif scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'S ' + str(scopes[scope_id][key][1])))
		elif scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'C ' + str(scopes[scope_id][key][1])))
		else:
			# scopes[scope_id][key][0] == NODE_TYPE_EXIT
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'E ' + str(scopes[scope_id][key][1]) + ' ' + str(scopes[scope_id][key][3])))
		node_mappings[key] = node_index

	for key in scopes[scope_id]:
		if scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
			if scopes[scope_id][key][3] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][3]]))
		# else scopes[scope_id][key][0] == NODE_TYPE_EXIT, do nothing

graph.write_png('solution.png')
