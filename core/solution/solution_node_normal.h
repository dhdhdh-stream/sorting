#ifndef SOLUTION_NODE_NORMAL_H
#define SOLUTION_NODE_NORMAL_H

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

	void reset() override;

	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
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
	void increment(SolutionNode* explore_node,
				   int& explore_type,
				   bool* potential_states_on) override;
};

#endif /* SOLUTION_NODE_NORMAL_H */