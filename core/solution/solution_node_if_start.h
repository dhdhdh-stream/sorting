#ifndef SOLUTION_NODE_IF_START_H
#define SOLUTION_NODE_IF_START_H

class SolutionNodeIfStart : public SolutionNode {
public:
	std::vector<SolutionNode*> children_nodes;
	std::vector<Network*> children_score_networks;
	std::vector<std::string> children_score_network_names;

	std::vector<bool> children_on;

	SolutionNodeIfEnd* end;

	std::vector<int> scope_states_on;

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
						   std::vector<SolutionNode*>& loop_scopes;
						   std::vector<int>& loop_scope_counts,
						   bool is_first_time,
						   int& iter_explore_type,
						   SolutionNode* iter_explore_node,
						   double* potential_state_vals,
						   bool* potential_states_on,
						   std::vector<NetworkHistory*>& network_historys,
						   std::vector<double>& guesses,
						   std::vector<int>& explore_decisions,
						   std::vector<double>& explore_diffs,
						   std::vector<bool>& explore_loop_decisions) override;
	void backprop(double score,
				  double misguess,
				  double* state_errors,
				  bool* states_on,
				  int& iter_explore_type,
				  SolutionNode* iter_explore_node,
				  double* potential_state_errors,
				  bool* potential_states_on,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions,
				  std::vector<double>& explore_diffs,
				  std::vector<bool>& explore_loop_decisions) override;

	void clear_potential_state() override;

private:
	void activate_children_networks(Problem& problem,
									double* state_vals,
									bool* states_on,
									bool backprop,
									std::vector<NetworkHistory*>& network_historys,
									double& best_score,
									int& best_index);
};

#endif /* SOLUTION_NODE_IF_START_H */