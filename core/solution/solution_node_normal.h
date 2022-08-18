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

	int explore_type;
	std::vector<SolutionNode*> explore_path;
	SolutionNode* explore_start_non_inclusive;
	SolutionNode* explore_start_inclusive;
	SolutionNode* explore_end_inclusive;
	SolutionNode* explore_end_non_inclusive;
	// reuse score_network_inputs_state_indexes
	Network* explore_if_network;
	Network* explore_halt_network;
	Network* explore_no_halt_network;

	void reset() override;

	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes;
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
};

#endif /* SOLUTION_NODE_NORMAL_H */