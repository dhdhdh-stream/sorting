#ifndef SOLUTION_NODE_LOOP_START_H
#define SOLUTION_NODE_LOOP_START_H

class SolutionNodeLoopStart : public SolutionNode {
public:
	std::vector<int> scope_states_on;

	SolutionNode* next;

	SolutionNode* previous;
	SolutionNode* loop_in;

	std::vector<int> current_explore_states;

	std::vector<SolutionNode*> nodes_directly_in_scope;

	void reset() override;

	SolutionNode* tune(Problem& problem,
					   double* state_vals,
					   bool* states_on,
					   std::vector<NetworkHistory*>& network_historys) override;
	void tune_update(double score,
					 double* state_errors,
					 bool* states_on,
					 std::vector<NetworkHistory*>& network_historys) override;

	void increment() override;

	void explore_setup_network_to_test_new_state();
};

#endif /* SOLUTION_NODE_LOOP_START_H */