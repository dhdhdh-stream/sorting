import pydot

# ACTION_NOOP = -1
# ACTION_LEFT = 0
# ACTION_RIGHT = 1
# ACTION_SWAP = 2
ACTION_NOOP = -1
ACTION_UP = 0
ACTION_RIGHT = 1
ACTION_DOWN = 2
ACTION_LEFT = 3
ACTION_CLICK = 4
ACTION_FLAG = 5
ACTION_DOUBLECLICK = 6

NODE_TYPE_ACTION = 0
NODE_TYPE_SCOPE = 1
NODE_TYPE_BRANCH = 2
NODE_TYPE_INFO_SCOPE = 3
NODE_TYPE_INFO_BRANCH = 4

scopes = {}
info_scopes = {}

file = open('../display.txt')

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
		original_next_node_id = int(file.readline())
		branch_next_node_id = int(file.readline())

		nodes[node_id] = [node_type,
						  original_next_node_id,
						  branch_next_node_id]
	elif node_type == NODE_TYPE_INFO_SCOPE:
		inner_scope_id = int(file.readline())

		next_node_id = int(file.readline())

		nodes[node_id] = [node_type,
						  inner_scope_id,
						  next_node_id]
	elif node_type == NODE_TYPE_INFO_BRANCH:
		inner_scope_id = int(file.readline())

		original_next_node_id = int(file.readline())
		branch_next_node_id = int(file.readline())

		nodes[node_id] = [node_type,
						  inner_scope_id,
						  original_next_node_id,
						  branch_next_node_id]

print(nodes)

scopes[-1] = nodes

num_scopes = int(file.readline())

for s_index in range(num_scopes):
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
		elif node_type == NODE_TYPE_BRANCH:
			original_next_node_id = int(file.readline())
			branch_next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  original_next_node_id,
							  branch_next_node_id]
		elif node_type == NODE_TYPE_INFO_SCOPE:
			inner_scope_id = int(file.readline())

			next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  inner_scope_id,
							  next_node_id]
		elif node_type == NODE_TYPE_INFO_BRANCH:
			inner_scope_id = int(file.readline())

			original_next_node_id = int(file.readline())
			branch_next_node_id = int(file.readline())

			nodes[node_id] = [node_type,
							  inner_scope_id,
							  original_next_node_id,
							  branch_next_node_id]

	print(nodes)

	scopes[s_index] = nodes

num_info_scopes = int(file.readline())

for i_index in range(num_info_scopes):
	state = int(file.readline())
	if state == 0:
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
			elif node_type == NODE_TYPE_BRANCH:
				original_next_node_id = int(file.readline())
				branch_next_node_id = int(file.readline())

				nodes[node_id] = [node_type,
								  original_next_node_id,
								  branch_next_node_id]
			elif node_type == NODE_TYPE_INFO_SCOPE:
				inner_scope_id = int(file.readline())

				next_node_id = int(file.readline())

				nodes[node_id] = [node_type,
								  inner_scope_id,
								  next_node_id]
			elif node_type == NODE_TYPE_INFO_BRANCH:
				inner_scope_id = int(file.readline())

				original_next_node_id = int(file.readline())
				branch_next_node_id = int(file.readline())

				nodes[node_id] = [node_type,
								  inner_scope_id,
								  original_next_node_id,
								  branch_next_node_id]

		print(nodes)

		info_scopes[i_index] = nodes

file.close()

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
	elif action == 6:
		result = 'DOUBLECLICK'
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

global_node_index = 0

for scope_id in scopes:
# for scope_id in [0]:
	node_mappings = {}
	for key in scopes[scope_id]:
		node_index = global_node_index
		global_node_index += 1
		if scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + pretty_print_action(scopes[scope_id][key][1])))
		elif scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'S ' + str(scopes[scope_id][key][1])))
		elif scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n'))
		elif scopes[scope_id][key][0] == NODE_TYPE_INFO_SCOPE:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'IS ' + str(scopes[scope_id][key][1])))
		elif scopes[scope_id][key][0] == NODE_TYPE_INFO_BRANCH:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'IB ' + str(scopes[scope_id][key][1])))
		node_mappings[key] = node_index

	for key in scopes[scope_id]:
		if scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			if scopes[scope_id][key][1] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][1]], style="dotted"))
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_INFO_SCOPE:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]]))
		elif scopes[scope_id][key][0] == NODE_TYPE_INFO_BRANCH:
			if scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][2]], style="dotted"))
			if scopes[scope_id][key][3] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[scopes[scope_id][key][3]]))

for scope_id in info_scopes:
# for scope_id in [0]:
	node_mappings = {}
	for key in info_scopes[scope_id]:
		node_index = global_node_index
		global_node_index += 1
		if info_scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + pretty_print_action(info_scopes[scope_id][key][1])))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'S ' + str(info_scopes[scope_id][key][1])))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n'))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_INFO_SCOPE:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'IS ' + str(info_scopes[scope_id][key][1])))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_INFO_BRANCH:
			graph.add_node(pydot.Node(node_index, label=str(scope_id) + ' ' + str(key) + '\n' + 'IB ' + str(info_scopes[scope_id][key][1])))
		node_mappings[key] = node_index

	for key in info_scopes[scope_id]:
		if info_scopes[scope_id][key][0] == NODE_TYPE_ACTION:
			if info_scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][2]]))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_SCOPE:
			if info_scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][2]]))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_BRANCH:
			if info_scopes[scope_id][key][1] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][1]], style="dotted"))
			if info_scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][2]]))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_INFO_SCOPE:
			if info_scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][2]]))
		elif info_scopes[scope_id][key][0] == NODE_TYPE_INFO_BRANCH:
			if info_scopes[scope_id][key][2] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][2]], style="dotted"))
			if info_scopes[scope_id][key][3] != -1:
				graph.add_edge(pydot.Edge(node_mappings[key], node_mappings[info_scopes[scope_id][key][3]]))

graph.write_png('solution.png')
