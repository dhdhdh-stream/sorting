#ifndef SOLUTION_NODE_ACTION_H
#define SOLUTION_NODE_ACTION_H

class SolutionNodeAction : public SolutionNode {
public:
	Action action;

	std::vector<std::vector<int>> state_network_inputs_state_indexes;
	std::vector<Network*> state_networks;
	std::vector<int> state_networks_target_states;
	std::vector<std::string> state_network_names;

	// reuse network_inputs_state_indexes
	std::vector<std::vector<int>> potential_potential_inputs_state_indexes;
	std::vector<Network*> potential_state_networks;
	std::vector<int> potential_state_networks_target_states;

	SolutionNode* next;

	SolutionNode* previous;

	SolutionNodeAction(SolutionNode* parent,
					   int node_index,
					   Action action);
	~SolutionNodeAction();

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
	void activate_state_networks_eval(Problem& problem,
									  double* state_vals,
									  bool* states_on,
									  bool backprop,
									  std::vector<NetworkHistory*>& network_historys);
	void activate_state_networks_with_potential(Problem& problem,
												double* state_vals,
												bool* states_on,
												double* potential_state_vals,
												bool* potential_states_on,
												bool backprop,
												std::vector<NetworkHistory*>& network_historys);

	void backprop_state_networks(double* state_errors,
								 bool* states_on,
								 std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_errors_with_no_weight_change(
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_with_potential(double* potential_state_errors,
												bool* potential_states_on,
												std::vector<NetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_ACTION_H */