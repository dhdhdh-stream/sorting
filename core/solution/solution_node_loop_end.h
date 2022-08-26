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

	Network* halt_network;

	Network* no_halt_network;

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

	void activate_networks(Problem& problem,
						   double* state_vals,
						   bool* states_on,
						   std::vector<SolutionNode*>& loop_scopes,
						   std::vector<int>& loop_scope_counts,
						   bool backprop,
						   std::vector<NetworkHistory*>& network_historys,
						   double& score,
						   bool& should_halt);
	void activate_networks_with_potential(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  std::vector<SolutionNode*>& loop_scopes,
										  std::vector<int>& loop_scope_counts,
										  double* potential_state_vals,
										  bool* potential_states_on,
										  bool backprop,
										  std::vector<NetworkHistory*>& network_historys,
										  double& score,
										  bool& should_halt);

	void backprop_networks(double score,
						   double* state_errors,
						   bool* states_on,
						   std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_networks_with_potential(double score,
										  double* potential_state_errors,
										  std::vector<NetworkHistory*>& network_historys);
	// don't need potential_states_on because information in network_history
};

#endif /* SOLUTION_NODE_LOOP_END_H */