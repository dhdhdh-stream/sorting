#ifndef SOLUTION_NODE_IF_END_H
#define SOLUTION_NODE_IF_END_H

class SolutionNodeIfEnd : public SolutionNode {
public:
	SolutionNode* next;

	SolutionNodeIfStart* start;

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

#endif /* SOLUTION_NODE_IF_END_H */