#ifndef SOLUTION_NODE_LOOP_START_H
#define SOLUTION_NODE_LOOP_START_H

// special case loop start loop

class SolutionNodeLoopStart : public SolutionNode {
public:
	std::vector<int> scope_states_on;

	SolutionNode* next;

	SolutionNode* previous;
	SolutionNode* loop_in;

	SolutionNodeLoopEnd* end;

	// std::vector<int> current_explore_states;

	// std::vector<SolutionNode*> nodes_directly_in_scope;

	void reset() override;

	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   int visited_count,
						   SolutionNode* explore_node,
						   int& explore_type,
						   double* potential_state_vals,
						   bool* potential_states_on,
						   std::vector<NetworkHistory*>& network_historys) override;
	void backprop(double score,
				  SolutionNode* explore_node,
				  int& explore_type,
				  double* potential_state_errors,
				  bool* potential_states_on,
				  std::vector<NetworkHistory*>& network_historys) override;

	// void explore_setup_network_to_test_new_state();
};

#endif /* SOLUTION_NODE_LOOP_START_H */