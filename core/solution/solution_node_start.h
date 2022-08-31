#ifndef SOLUTION_NODE_START_H
#define SOLUTION_NODE_START_H

#include "solution_node.h"

class SolutionNodeStart : public SolutionNode {
public:
	std::vector<int> start_states;

	SolutionNode* next;

	SolutionNodeStart(Solution* solution);
	SolutionNodeStart(Solution* solution,
					  int node_index,
					  std::ifstream& save_file);
	~SolutionNodeStart();

	void reset() override;

	void add_potential_state(std::vector<int> potential_state_indexes,
							 SolutionNode* explore_node) override;
	void extend_with_potential_state(std::vector<int> potential_state_indexes,
									 std::vector<int> new_state_indexes,
									 SolutionNode* explore_node) override;
	void delete_potential_state(std::vector<int> potential_state_indexes,
								SolutionNode* explore_node) override;
	void clear_potential_state() override;

	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   std::vector<bool>& loop_decisions,
						   int& iter_explore_type,
						   SolutionNode*& iter_explore_node,
						   IterExplore*& iter_explore,
						   double* potential_state_vals,
						   std::vector<int>& potential_state_indexes,
						   std::vector<NetworkHistory*>& network_historys,
						   std::vector<std::vector<double>>& guesses,
						   std::vector<int>& explore_decisions,
						   bool save_for_display,
						   std::ofstream& display_file) override;
	void backprop(double score,
				  double misguess,
				  double* state_errors,
				  bool* states_on,
				  std::vector<bool>& loop_decisions,
				  int& iter_explore_type,
				  SolutionNode*& iter_explore_node,
				  double* potential_state_errors,
				  std::vector<int>& potential_state_indexes,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions) override;

	void save(std::ofstream& save_file) override;
	void save_for_display(std::ofstream& save_file) override;
};

#endif /* SOLUTION_NODE_START_H */