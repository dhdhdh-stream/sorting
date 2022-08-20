#ifndef SOLUTION_NODE_IF_START_H
#define SOLUTION_NODE_IF_START_H

// special case if start loop

class SolutionNodeIfStart : public SolutionNode {
public:
	std::vector<SolutionNode*> children_nodes;
	std::vector<Network*> children_score_networks;
	std::vector<std::string> children_score_network_names;

	std::vector<bool> children_on;

	SolutionNodeIfEnd* end;

	std::vector<int> scope_states_on;

	// potential_scope_states

	// without folds, explore states once?

	SolutionNode* previous;

	void reset() override;

	void add_potential_state(std::vector<int> potential_state_indexes,
							 SolutionNode* scope) override;
	void extend_with_potential_state(std::vector<int> potential_state_indexes,
									 std::vector<int> new_state_indexes,
									 SolutionNode* scope) override;
	void reset_potential_state(std::vector<int> potential_state_indexes,
							   SolutionNode* scope) override;

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

	void explore_increment(double score) override;

	void clear_potential_state() override;
	void clear_explore() override;
};

#endif /* SOLUTION_NODE_IF_START_H */