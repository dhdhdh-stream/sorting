#ifndef SOLUTION_NODE_LOOP_START_H
#define SOLUTION_NODE_LOOP_START_H

#include "solution_node.h"
#include "solution_node_loop_end.h"

class SolutionNodeLoopEnd;
class SolutionNodeLoopStart : public SolutionNode {
public:
	std::vector<int> scope_states_on;

	SolutionNode* next;

	SolutionNode* previous;

	SolutionNodeLoopEnd* end;

	SolutionNodeLoopStart(Solution* solution);
	SolutionNodeLoopStart(SolutionNode* parent,
						  int node_index);
	SolutionNodeLoopStart(Solution* solution,
						  int node_index,
						  std::ifstream& save_file);
	~SolutionNodeLoopStart();

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
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   bool is_first_time,
						   int& iter_explore_type,
						   SolutionNode*& iter_explore_node,
						   double* potential_state_vals,
						   bool* potential_states_on,
						   std::vector<NetworkHistory*>& network_historys,
						   std::vector<double>& guesses,
						   std::vector<int>& explore_decisions,
						   std::vector<double>& explore_diffs,
						   std::vector<bool>& explore_loop_decisions,
						   bool save_for_display,
						   std::ofstream& display_file) override;
	void backprop(double score,
				  double misguess,
				  double* state_errors,
				  bool* states_on,
				  int& iter_explore_type,
				  SolutionNode*& iter_explore_node,
				  double* potential_state_errors,
				  bool* potential_states_on,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions,
				  std::vector<double>& explore_diffs,
				  std::vector<bool>& explore_loop_decisions) override;

	void clear_potential_state() override;

	void save(std::ofstream& save_file) override;
	void save_for_display(std::ofstream& save_file) override;
};

#endif /* SOLUTION_NODE_LOOP_START_H */