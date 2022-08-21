#ifndef SOLUTION_NODE_LOOP_END_H
#define SOLUTION_NODE_LOOP_END_H

class SolutionNodeLoopEnd : public SolutionNode {
public:
	SolutionNode* next;

	std::vector<int> halt_networks_inputs_state_indexes;
	std::vector<int> halt_networks_potential_inputs_state_indexes;

	Network* halt_network;
	std::string halt_network_name;

	Network* no_halt_network;
	std::string no_halt_network_name;

	SolutionNodeLoopStart* start;

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
	void activate_networks(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   bool backprop,
						   std::vector<NetworkHistory*>& network_historys,
						   double& score,
						   bool& should_halt);
	void activate_networks_with_potential(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  double* potential_state_vals,
										  bool* potential_states_on,
										  bool backprop,
										  std::vector<NetworkHistory*>& network_historys,
										  double& score,
										  bool& should_halt);

	void backprop_networks(double score,
						   double* state_errors,
						   bool* states_on,
						   std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_with_potential(double score,
										  double* potential_state_errors,
										  vector<NetworkHistory*>& network_historys);
	// don't need potential_states_on because information in network_history
};

#endif /* SOLUTION_NODE_LOOP_END_H */