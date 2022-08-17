#ifndef SOLUTION_NODE_IF_START_H
#define SOLUTION_NODE_IF_START_H

class SolutionNodeIfStart : public SolutionNode {
public:
	std::vector<SolutionNode*> children_nodes;
	std::vector<int> network_inputs_state_indexes;
	std::vector<int> network_inputs_potential_state_indexes;
	std::vector<Network*> children_score_networks;
	std::vector<std::string> children_score_network_names;

	std::vector<bool> children_on;

	SolutionNodeIfEnd* end;

	std::vector<int> scope_states_on;

	// potential_scope_states

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

#endif /* SOLUTION_NODE_IF_START_H */