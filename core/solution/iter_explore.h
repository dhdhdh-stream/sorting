#ifndef ITER_EXPLORE_H
#define ITER_EXPLORE_H

const int ITER_EXPLORE_TYPE_EXPLORE = 0;
const int ITER_EXPLORE_TYPE_LEARN_FLAT = 1;
const int ITER_EXPLORE_TYPE_MEASURE_FLAT = 2;
const int ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH = 3;
const int ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH = 4;
const int ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH = 5;
const int ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE = 6;
const int ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE = 7;
const int ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE = 8;

const int ITER_EXPLORE_LOCAL_TYPE_JUMP_IF = 1;
const int ITER_EXPLORE_LOCAL_TYPE_JUMP_OUT = 2;

class IterExplore {
public:
	SolutionNode* explore_node;
	int type;
	
	int local_type;

	std::vector<SolutionNode*> explore_path;

	int parent_jump_scope_start_non_inclusive_index;
	int parent_jump_end_non_inclusive_index;

	int jump_end_non_inclusive_index;
	
	// int if_type;
	// int child_index;

	std::vector<SolutionNode*> scopes;
	std::vector<int> scope_states;
	std::vector<int> scope_locations;

	IterExplore(SolutionNode* explore_node,
				int type) {
		this->explore_node = explore_node;
		this->type = type;
	};
	~IterExplore() {
		// do nothing
	};
};

#endif /* ITER_EXPLORE_H */