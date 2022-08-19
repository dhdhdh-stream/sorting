#ifndef SOLUTION_NODE_NORMAL_H
#define SOLUTION_NODE_NORMAL_H

const int NODE_NORMAL_EXPLORE_TYPE_JUMP = 0;
const int NODE_NORMAL_EXPLORE_TYPE_LOOP = 1;

class SolutionNodeNormal : public SolutionNode {
public:
	Action action;

	std::vector<std::vector<int>> state_network_inputs_state_indexes;
	std::vector<Network*> state_networks;
	std::vector<int> state_networks_target_states;
	std::vector<std::string> state_network_names;

	std::vector<std::vector<int>> potential_inputs_state_indexes;
	std::vector<std::vector<int>> potential_potential_inputs_state_indexes;
	std::vector<Network*> potential_state_networks;
	std::vector<int> potential_state_networks_target_states;

	SolutionNode* next;

	SolutionNode* previous;

	int node_explore_type;
	std::vector<SolutionNode*> explore_path;
	SolutionNode* explore_start_non_inclusive;
	SolutionNode* explore_start_inclusive;
	SolutionNode* explore_end_inclusive;
	SolutionNode* explore_end_non_inclusive;
	// reuse score_network_inputs_state_indexes
	Network* explore_if_network;
	Network* explore_halt_network;
	Network* explore_no_halt_network;
	double explore_diff;
	// TODO: compare misguess

	void reset() override;

	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes;
						   std::vector<int>& loop_scope_counts,
						   bool is_first_time,
						   int& explore_type,
						   SolutionNode* explore_node,
						   double* potential_state_vals,
						   bool* potential_states_on,
						   std::vector<NetworkHistory*>& network_historys,
						   std::vector<double>& guesses,
						   std::vector<int>& explore_decisions) override;
	void backprop(double score,
				  int& explore_type,
				  SolutionNode* explore_node,
				  double* potential_state_errors,
				  bool* potential_states_on,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions) override;

	void explore_increment(double score) override;

protected:
	void activate_state_networks(Problem& problem,
								 double* state_vals,
								 bool* states_on);
	void activate_state_networks_eval(Problem& problem,
									  double* state_vals,
									  bool* states_on,
									  std::vector<NetworkHistory*>& network_historys);
	// activate_state_networks_path identical to activate_state_networks_eval
	void activate_state_networks_state(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   double* potential_state_vals,
									   bool* potential_states_on,
									   std::vector<NetworkHistory*>& network_historys);

	void backprop_state_networks_eval(double* state_errors,
									  bool* states_on,
									  std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_path(double* state_errors,
									  bool* states_on,
									  std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_state(double* potential_state_errors,
									   bool* potential_states_on,
									   std::vector<NetworkHistory*>& network_historys);

	double activate_explore_if_network(Problem& problem,
									   double* state_vals,
									   bool* states_on);
	void activate_explore_if_network_eval(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_if_network_eval(double score,
										  double* state_errors,
										  bool* states_on,
										  std::vector<NetworkHistory*>& network_historys);

	double activate_explore_halt_network(Problem& problem,
										 double* state_vals,
										 bool* states_on);
	void activate_explore_halt_network_eval(Problem& problem,
											double* state_vals,
											bool* states_on,
											std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_halt_network_eval(double score,
											double* state_errors,
											bool* states_on,
											std::vector<NetworkHistory*>& network_historys);

	double activate_explore_no_halt_network(Problem& problem,
											double* state_vals,
											bool* states_on);
	void activate_explore_no_halt_network_eval(Problem& problem,
											   double* state_vals,
											   bool* states_on,
											   std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_no_halt_network_eval(double score,
											   double* states_errors,
											   bool* states_on,
											   std::vector<NetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_NORMAL_H */