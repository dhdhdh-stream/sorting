import pydot

LEFT = 0
STAY = 1
RIGHT = 2

NODE_TYPE_EMPTY = 0;
NODE_TYPE_ACTION = 1;
NODE_TYPE_START_SCOPE = 2;
NODE_TYPE_JUMP_SCOPE = 3;

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

graph = pydot.Dot(graph_type='digraph', strict=True)

def parse_node(file, node_type):
	if node_type == NODE_TYPE_EMPTY:
		return [NODE_TYPE_EMPTY]
	elif node_type == NODE_TYPE_ACTION:
		write = float(file.readline())
		move = int(file.readline())
		return [NODE_TYPE_ACTION, [write, move]]
	else:
		# node_type == NODE_TYPE_JUMP_SCOPE
		top_path_size = int(file.readline())
		top_path = []
		for n_index in range(top_path_size):
			path_node_type = int(file.readline())
			path_node = parse_node(file, path_node_type)
			top_path.append(path_node)
		num_child = int(file.readline())
		child_paths = []
		for c_index in range(num_child):
			child_path_size = int(file.readline())
			child_path = []
			for n_index in range(child_path_size):
				path_node_type = int(file.readline())
				path_node = parse_node(file, path_node_type)
				child_path.append(path_node)
			child_paths.append(child_path)
		return [NODE_TYPE_JUMP_SCOPE, top_path, child_paths]

start_scope_path = []
with open('../solution.txt') as file:
	path_size = int(file.readline())
	for n_index in range(path_size):
		path_node_type = int(file.readline())
		path_node = parse_node(file, path_node_type)
		start_scope_path.append(path_node)

global_node_index = 0

def build_graph(node):
	global global_node_index

	if node[0] == NODE_TYPE_EMPTY:
		node_index = global_node_index
		global_node_index += 1
		display_node = pydot.Node(node_index, label='E')
		graph.add_node(display_node)
		return node_index, node_index
	elif node[0] == NODE_TYPE_ACTION:
		node_index = global_node_index
		global_node_index += 1
		display_node = pydot.Node(node_index, label=pretty_print_action(node[1]))
		graph.add_node(display_node)
		return node_index, node_index
	else:
		# node[0] == NODE_TYPE_JUMP_SCOPE
		start_node_index = global_node_index
		global_node_index += 1
		if_node_index = global_node_index
		global_node_index += 1
		end_node_index = global_node_index
		global_node_index += 1
		start_display_node = pydot.Node(start_node_index, label='S')
		graph.add_node(start_display_node)
		if_display_node = pydot.Node(if_node_index, label='I')
		graph.add_node(if_display_node)
		end_display_node = pydot.Node(end_node_index, label='E')
		graph.add_node(end_display_node)

		if len(node[1]) == 0:
			edge = pydot.Edge(start_node_index, if_node_index)
			graph.add_edge(edge)
		else:
			curr_path_node_start_index, curr_path_node_end_index = build_graph(node[1][0])
			edge = pydot.Edge(start_node_index, curr_path_node_start_index)
			graph.add_edge(edge)
			for n_index in range(1, len(node[1])):
				next_path_node_start_index, next_path_node_end_index = build_graph(node[1][n_index])
				edge = pydot.Edge(curr_path_node_end_index, next_path_node_start_index)
				graph.add_edge(edge)
				curr_path_node_start_index = next_path_node_start_index
				curr_path_node_end_index = next_path_node_end_index
			edge = pydot.Edge(curr_path_node_end_index, if_node_index)
			graph.add_edge(edge)

		for c_index in range(len(node[2])):
			if len(node[2][c_index]) == 0:
				edge = pydot.Edge(if_node_index, end_node_index)
				graph.add_edge(edge)
			else:
				curr_path_node_start_index, curr_path_node_end_index = build_graph(node[2][c_index][0])
				edge = pydot.Edge(if_node_index, curr_path_node_start_index)
				graph.add_edge(edge)
				for n_index in range(1, len(node[2][c_index])):
					next_path_node_start_index, next_path_node_end_index = build_graph(node[2][c_index][n_index])
					edge = pydot.Edge(curr_path_node_end_index, next_path_node_start_index)
					graph.add_edge(edge)
					curr_path_node_start_index = next_path_node_start_index
					curr_path_node_end_index = next_path_node_end_index
				edge = pydot.Edge(curr_path_node_end_index, end_node_index)
				graph.add_edge(edge)

		return start_node_index, end_node_index

overall_start_index = global_node_index
global_node_index += 1
overall_end_index = global_node_index
global_node_index += 1
overall_start_node = pydot.Node(overall_start_index, label='S')
graph.add_node(overall_start_node)
overall_end_node = pydot.Node(overall_end_index, label='E')
graph.add_node(overall_end_node)
if len(start_scope_path) == 0:
	edge = pydot.Edge(overall_start_index, overall_end_index)
	graph.add_edge(edge)
else:
	curr_path_node_start_index, curr_path_node_end_index = build_graph(start_scope_path[0])
	edge = pydot.Edge(overall_start_index, curr_path_node_start_index)
	graph.add_edge(edge)
	for n_index in range(1, len(start_scope_path)):
		next_path_node_start_index, next_path_node_end_index = build_graph(start_scope_path[n_index])
		edge = pydot.Edge(curr_path_node_end_index, next_path_node_start_index)
		graph.add_edge(edge)
		curr_path_node_start_index = next_path_node_start_index
		curr_path_node_end_index = next_path_node_end_index
	edge = pydot.Edge(curr_path_node_end_index, overall_end_index)
	graph.add_edge(edge)

graph.write_png('solution.png')
