#ifndef SOLUTION_NODE_LOOP_END_H
#define SOLUTION_NODE_LOOP_END_H

#include "solution_node.h"
#include "solution_node_loop_start.h"

class SolutionNodeLoopStart;
class SolutionNodeLoopEnd : public SolutionNode {
public:
	SolutionNode* next;

	std::vector<int> halt_networks_inputs_state_indexes;
	std::vector<int> halt_networks_potential_inputs_state_indexes;

	Network* halt_score_network;
	Network* halt_certainty_network;

	Network* no_halt_score_network;
	Network* no_halt_certainty_network;

	SolutionNodeLoopStart* start;

	SolutionNodeLoopEnd(Solution* solution);
	SolutionNodeLoopEnd(SolutionNode* parent,
						int node_index);
	SolutionNodeLoopEnd(Solution* solution,
						int node_index,
						std::ifstream& save_file);
	~SolutionNodeLoopEnd();

	void reset() override;

	void add_potential_state(std::vector<int> potential_state_indexes,
							 SolutionNode* explore_node) override;
	void extend_with_potential_state(std::vector<int> potential_state_indexes,
									 std::vector<int> new_state_indexes,
									 SolutionNode* explore_node) override;
	void delete_potential_state(std::vector<int> potential_state_indexes,
								SolutionNode* explore_node) override;
	void clear_potential_state() override;

	// SolutionNode* activate(Problem& problem,
	// 					   double* state_vals,
	// 					   bool* states_on,
	// 					   std::vector<SolutionNode*>& loop_scopes,
	// 					   std::vector<int>& loop_scope_counts,
	// 					   int& iter_explore_type,
	// 					   SolutionNode*& iter_explore_node,
	// 					   IterExplore*& iter_explore,
	// 					   double* potential_state_vals,
	// 					   std::vector<int>& potential_state_indexes,
	// 					   std::vector<NetworkHistory*>& network_historys,
	// 					   std::vector<std::vector<double>>& guesses,
	// 					   std::vector<int>& explore_decisions,
	// 					   std::vector<bool>& explore_loop_decisions,
	// 					   bool save_for_display,
	// 					   std::ofstream& display_file) override;
	SolutionNode* activate(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   int& iter_explore_type,
						   SolutionNode*& iter_explore_node,
						   IterExplore*& iter_explore,
						   double* potential_state_vals,
						   std::vector<int>& potential_state_indexes,
						   std::vector<NetworkHistory*>& network_historys,
						   std::vector<std::vector<double>>& guesses,
						   std::vector<int>& explore_decisions,
						   std::vector<bool>& explore_loop_decisions) override;
	void backprop(double score,
				  double misguess,
				  double* state_errors,
				  bool* states_on,
				  int& iter_explore_type,
				  SolutionNode*& iter_explore_node,
				  double* potential_state_errors,
				  std::vector<int>& potential_state_indexes,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions,
				  std::vector<bool>& explore_loop_decisions) override;

	void save(std::ofstream& save_file) override;
	void save_for_display(std::ofstream& save_file) override;

	void activate_networks(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   bool backprop,
						   bool is_first_explore,
						   std::vector<NetworkHistory*>& network_historys,
						   bool& should_halt);
	void activate_networks_with_potential(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  std::vector<SolutionNode*>& loop_scopes,
										  std::vector<int>& loop_scope_counts,
										  double* potential_state_vals,
										  std::vector<int>& potential_state_indexes,
										  bool backprop,
										  bool is_first_explore,
										  std::vector<NetworkHistory*>& network_historys,
										  bool& should_halt);

	void backprop_networks(double score,
						   double misguess,
						   double* state_errors,
						   bool* states_on,
						   std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_errors_with_no_weight_change(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_with_potential(double score,
										  double misguess,
										  double* potential_state_errors,
										  std::vector<NetworkHistory*>& network_historys);
	// don't need potential_states_indexes because information in network_history
};

#endif /* SOLUTION_NODE_LOOP_END_H */