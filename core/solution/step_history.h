#ifndef STEP_HISTORY_H
#define STEP_HISTORY_H

class StepHistory {
public:
	SolutionNode* node_visited;
	
	bool action_performed;
	double previous_observations;

	int end_scope_status;

	bool is_explore_callback;

	StepHistory(SolutionNode* node_visited,
				bool action_performed,
				double previous_observations,
				int end_scope_status,
				bool is_explore_callback) {
		this->node_visited = node_visited;
		this->action_performed = action_performed;
		this->previous_observations = previous_observations;
		this->end_scope_status = end_scope_status;
		this->is_explore_callback = is_explore_callback;
	};
	// no user-declared destructor for implicit move constructor
};

#endif /* STEP_HISTORY_H */