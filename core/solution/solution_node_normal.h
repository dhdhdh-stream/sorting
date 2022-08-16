#ifndef SOLUTION_NODE_NORMAL_H
#define SOLUTION_NODE_NORMAL_H

class SolutionNodeNormal : public SolutionNode {
public:
	Action action;

	std::vector<std::vector<int>> state_network_inputs_state_indexes;
	std::vector<Network*> state_networks;
	std::vector<int> state_networks_target_states;
	std::vector<std::vector<int>> state_networks_backprop_indexes;
	std::vector<std::string> state_network_names;

	SolutionNode* next;

	SolutionNode* previous;

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
};

#endif /* SOLUTION_NODE_NORMAL_H */