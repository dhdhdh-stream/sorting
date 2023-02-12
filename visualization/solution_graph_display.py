import pydot

ACTION_START = -1
ACTION_LEFT = 0
ACTION_STAY = 1
ACTION_RIGHT = 2

STEP_TYPE_STEP = 0
STEP_TYPE_BRANCH = 1
STEP_TYPE_FOLD = 2

def parse_scope(file):
	curr_id = int(file.readline())

	sequence_length = int(file.readline())

	scope = [curr_id,
			 [[-1,		# 0: is_inner_scope
			   -1,		# 1: scope_id
			   None,	# 2: scope
			   None,	# 3: action
			   -1,		# 4: step_type
			   None,	# 5: branch
			   None,	# 6: fold
			   0.0,		# 7: average_inner_scope_impact
			   0.0,		# 8: average_local_impact
			   0.0,		# 9: average_inner_branch_impact
			  ] for _ in range(sequence_length)]]

	for a_index in range(sequence_length):
		is_inner_scope = int(file.readline())
		scope[1][a_index][0] = is_inner_scope

		if is_inner_scope == 1:
			scope_id = int(file.readline())
			scope[1][a_index][1] = scope_id

			if scope_id > curr_id:
				new_scope = parse_scope(file)
				scope[1][a_index][2] = new_scope
		else:
			write = float(file.readline())
			move = int(file.readline())
			scope[1][a_index][3] = [write, move]

	for a_index in range(sequence_length):
		step_type = int(file.readline())
		scope[1][a_index][4] = step_type

		if step_type == STEP_TYPE_BRANCH:
			new_branch = parse_branch(file, curr_id)
			scope[1][a_index][5] = new_branch
		elif step_type == STEP_TYPE_FOLD:
			new_fold = parse_fold(file)
			scope[1][a_index][6] = new_fold

	for a_index in range(sequence_length):
		average_inner_scope_impact = float(file.readline())
		scope[1][a_index][7] = average_inner_scope_impact
		average_local_impact = float(file.readline())
		scope[1][a_index][8] = average_local_impact
		average_inner_branch_impact = float(file.readline())
		scope[1][a_index][9] = average_inner_branch_impact

	return scope

def parse_branch(file, curr_scope_id):
	num_branches = int(file.readline())

	branch = [[-1,		# 0: is_branch
			   None,	# 1: branch_path
			   None,	# 2: fold
			  ] for _ in range(num_branches)]

	for b_index in range(num_branches):
		is_branch = int(file.readline())
		branch[b_index][0] = is_branch

		if is_branch == 1:
			new_branch_path = parse_branch_path(file, curr_scope_id)
			branch[b_index][1] = new_branch_path
		else:
			new_fold = parse_fold(file)
			branch[b_index][2] = new_fold

	return branch

def parse_branch_path(file, curr_scope_id):
	sequence_length = int(file.readline())

	branch_path = [[-1,		# 0: is_inner_scope
					-1,		# 1: scope_id
					None,	# 2: scope
					None,	# 3: action
					-1,		# 4: step_type
					None,	# 5: branch
					None,	# 6: fold
					0.0,	# 7: average_inner_scope_impact
					0.0,	# 8: average_local_impact
					0.0,	# 9: average_inner_branch_impact
				   ] for _ in range(sequence_length)]

	for a_index in range(sequence_length):
		is_inner_scope = int(file.readline())
		branch_path[a_index][0] = is_inner_scope

		if is_inner_scope == 1:
			scope_id = int(file.readline())
			branch_path[a_index][1] = scope_id

			if scope_id > curr_scope_id:
				new_scope = parse_scope(file)
				branch_path[a_index][2] = new_scope
		else:
			write = float(file.readline())
			move = int(file.readline())
			branch_path[a_index][3] = [write, move]

	for a_index in range(sequence_length):
		step_type = int(file.readline())
		branch_path[a_index][4] = step_type

		if step_type == STEP_TYPE_BRANCH:
			new_branch = parse_branch(file, curr_id)
			branch_path[a_index][5] = new_branch
		elif step_type == STEP_TYPE_FOLD:
			new_fold = parse_fold(file)
			branch_path[a_index][6] = new_fold

	for a_index in range(sequence_length):
		average_inner_scope_impact = float(file.readline())
		branch_path[a_index][7] = average_inner_scope_impact
		average_local_impact = float(file.readline())
		branch_path[a_index][8] = average_local_impact
		average_inner_branch_impact = float(file.readline())
		branch_path[a_index][9] = average_inner_branch_impact

	return branch_path

def parse_fold(file):
	sequence_length = int(file.readline())

	fold = [[-1,	# 0: is_inner_scope
			 -1,	# 1: scope_id
			 None,	# 2: action
			] for _ in range(sequence_length)]

	for f_index in range(sequence_length):
		is_inner_scope = int(file.readline())
		fold[f_index][0] = is_inner_scope

		if is_inner_scope == 1:
			scope_id = int(file.readline())
			fold[f_index][1] = scope_id
		else:
			write = float(file.readline())
			move = int(file.readline())
			fold[f_index][2] = [write, move]

	return fold

file = open('../display.txt')
root = parse_scope(file)
file.close()

print(root)

def pretty_print_action(action):
	result = '('
	result += "{:.2f}".format(action[0])
	result += ', '
	if action[1] == -1:
		result += 'START'
	elif action[1] == 0:
		result += 'LEFT'
	elif action[1] == 1:
		result += 'STAY'
	elif action[1] == 2:
		result += 'RIGHT'
	result += ')'
	return result

graph = pydot.Dot(graph_type='digraph', strict=True)

global_node_index = 0

def build_scope(scope,
				start_node_index):
	global global_node_index

	curr_node_index = start_node_index

	scope_start_node_index = global_node_index
	global_node_index += 1
	scope_start_node = pydot.Node(scope_start_node_index, label='S'+str(scope[0])+' START')
	graph.add_node(scope_start_node)

	new_edge = pydot.Edge(curr_node_index, scope_start_node_index)
	graph.add_edge(new_edge)

	curr_node_index = scope_start_node_index

	for step in scope[1]:
		if step[0] == 1:
			if step[1] > scope[0]:
				end_node_index = build_scope(step[2],
											 curr_node_index)
				curr_node_index = end_node_index
			else:
				new_node_index = global_node_index
				global_node_index += 1
				new_node = pydot.Node(new_node_index, label='S'+str(step[1]))
				graph.add_node(new_node)

				new_edge = pydot.Edge(curr_node_index, new_node_index)
				graph.add_edge(new_edge)

				curr_node_index = new_node_index
		else:
			new_node_index = global_node_index
			global_node_index += 1
			new_node = pydot.Node(new_node_index, label=pretty_print_action(step[3]))
			graph.add_node(new_node)

			new_edge = pydot.Edge(curr_node_index, new_node_index)
			graph.add_edge(new_edge)

			curr_node_index = new_node_index

		if step[4] == STEP_TYPE_BRANCH:
			end_node_index = build_branch(step[5],
										  scope[0],
										  curr_node_index)
			curr_node_index = end_node_index
		elif step[5] == STEP_TYPE_FOLD:
			end_node_index = build_fold(step[6],
										curr_node_index)
			curr_node_index = end_node_index

	scope_end_node_index = global_node_index
	global_node_index += 1
	scope_end_node = pydot.Node(scope_end_node_index, label='S'+str(scope[0])+' END')
	graph.add_node(scope_end_node)

	new_edge = pydot.Edge(curr_node_index, scope_end_node_index)
	graph.add_edge(new_edge)

	curr_node_index = scope_end_node_index

	return curr_node_index

def build_branch(branch,
				 curr_scope_id,
				 start_node_index):
	global global_node_index

	merge_node_index = global_node_index
	global_node_index += 1
	merge_node = pydot.Node(merge_node_index, label='')
	graph.add_node(merge_node)

	for b in branch:
		if b[0] == 1:
			end_node_index = build_branch_path(b[1],
											   curr_scope_id,
											   start_node_index)

			new_edge = pydot.Edge(end_node_index, merge_node_index)
			graph.add_edge(new_edge)
		else:
			end_node_index = build_fold(b[2],
										start_node_index)

			new_edge = pydot.Edge(end_node_index, merge_node_index)
			graph.add_edge(new_edge)

	return merge_node_index

def build_branch_path(branch_path,
					  curr_scope_id,
					  start_node_index):
	global global_node_index

	curr_node_index = start_node_index

	for step in branch_path:
		if step[0] == 1:
			if step[1] > curr_scope_id:
				end_node_index = build_scope(step[2],
											 curr_node_index)
				curr_node_index = end_node_index
			else:
				new_node_index = global_node_index
				global_node_index += 1
				new_node = pydot.Node(new_node_index, label='S'+str(step[1]))
				graph.add_node(new_node)

				new_edge = pydot.Edge(curr_node_index, new_node_index)
				graph.add_edge(new_edge)

				curr_node_index = new_node_index
		else:
			new_node_index = global_node_index
			global_node_index += 1
			new_node = pydot.Node(new_node_index, label=pretty_print_action(step[3]))
			graph.add_node(new_node)

			new_edge = pydot.Edge(curr_node_index, new_node_index)
			graph.add_edge(new_edge)

			curr_node_index = new_node_index

		if step[4] == STEP_TYPE_BRANCH:
			end_node_index = build_branch(step[5],
										  curr_scope_id,
										  curr_node_index)
			curr_node_index = end_node_index
		elif step[5] == STEP_TYPE_FOLD:
			end_node_index = build_fold(step[6],
										curr_node_index)
			curr_node_index = end_node_index

	return curr_node_index

def build_fold(fold,
			   start_node_index):
	global global_node_index

	curr_node_index = start_node_index

	for step in fold:
		if step[0] == 1:
			new_node_index = global_node_index
			global_node_index += 1
			new_node = pydot.Node(new_node_index, label='S'+str(step[1]))
			graph.add_node(new_node)

			new_edge = pydot.Edge(curr_node_index, new_node_index)
			graph.add_edge(new_edge)

			curr_node_index = new_node_index
		else:
			new_node_index = global_node_index
			global_node_index += 1
			new_node = pydot.Node(new_node_index, label=pretty_print_action(step[2]))
			graph.add_node(new_node)

			new_edge = pydot.Edge(curr_node_index, new_node_index)
			graph.add_edge(new_edge)

			curr_node_index = new_node_index

	return curr_node_index

root_node_index = global_node_index
global_node_index += 1
root_node = pydot.Node(root_node_index, label='START')
graph.add_node(root_node)

build_scope(root, root_node_index)

graph.write_png('solution.png')
