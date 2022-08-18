#ifndef SOLUTION_NODE_LOOP_END_H
#define SOLUTION_NODE_LOOP_END_H

// special case halt loop

class SolutionNodeLoopEnd : public SolutionNode {
public:
	SolutionNode* halt;
	SolutionNode* no_halt;

	std::vector<int> network_inputs_state_indexes;
	std::vector<int> network_inputs_potential_state_indexes;

	Network* halt_network;
	std::string halt_network_name;

	Network* no_halt_network;
	std::string no_halt_network_name;

	SolutionNodeLoopStart* start;

	void reset() override;

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
};

#endif /* SOLUTION_NODE_LOOP_END_H */