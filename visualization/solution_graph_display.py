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
			   0.0,		# 7: average_score
			   0.0,		# 8: average_misguess
			   0.0,		# 9: average_inner_scope_impact
			   0.0,		# 10: average_local_impact
			   0.0,		# 11: average_inner_branch_impact
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
		average_score = float(file.readline())
		scope[1][a_index][7] = average_score
		average_misguess = float(file.readline())
		scope[1][a_index][8] = average_misguess
		average_inner_scope_impact = float(file.readline())
		scope[1][a_index][9] = average_inner_scope_impact
		average_local_impact = float(file.readline())
		scope[1][a_index][10] = average_local_impact
		average_inner_branch_impact = float(file.readline())
		scope[1][a_index][11] = average_inner_branch_impact

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
					0.0,	# 7: average_score
					0.0,	# 8: average_misguess
					0.0,	# 9: average_inner_scope_impact
					0.0,	# 10: average_local_impact
					0.0,	# 11: average_inner_branch_impact
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
		average_score = float(file.readline())
		branch_path[a_index][7] = average_score
		average_misguess = float(file.readline())
		branch_path[a_index][8] = average_misguess
		average_inner_scope_impact = float(file.readline())
		branch_path[a_index][9] = average_inner_scope_impact
		average_local_impact = float(file.readline())
		branch_path[a_index][10] = average_local_impact
		average_inner_branch_impact = float(file.readline())
		branch_path[a_index][11] = average_inner_branch_impact

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
