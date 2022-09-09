#ifndef EXPLORE_STEP_HISTORY_H
#define EXPLORE_STEP_HISTORY_H

class ExploreStepHistory {
public:
	SolutionNode* node_visited;
	
	bool action_performed;
	double previous_observations;

	int scope_state;

	int explore_decision;

	bool is_explore_callback;

	ExploreStepHistory(SolutionNode* node_visited,
					   bool action_performed,
					   double previous_observations,
					   int scope_state,
					   int explore_decision,
					   bool is_explore_callback) {
		this->node_visited = node_visited;
		this->action_performed = action_performed;
		this->previous_observations = previous_observations;
		this->scope_state = scope_state;
		this->explore_decision = explore_decision;
		this->is_explore_callback = is_explore_callback;
	};
	// no user-declared destructor for implicit move constructor(?)
};

#endif /* EXPLORE_STEP_HISTORY_H */