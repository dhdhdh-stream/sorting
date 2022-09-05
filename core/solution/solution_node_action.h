#ifndef SOLUTION_NODE_ACTION_H
#define SOLUTION_NODE_ACTION_H

#include <vector>

#include "action.h"
#include "solution_node.h"

class SolutionNodeAction : public SolutionNode {
public:
	Action action;

	std::vector<int> state_network_local_index;
	std::vector<Network*> state_networks;

	SolutionNodeAction(Solution* solution,
					   int node_index,
					   Action action,
					   std::vector<int> available_state);
	SolutionNodeAction(Solution* solution,
					   int node_index,
					   std::ifstream& save_file);
	~SolutionNodeAction();

	void reset() override;

	void add_potential_state(std::vector<int> potential_state_indexes,
							 SolutionNode* explore_node) override;
	void extend_with_potential_state(std::vector<int> potential_state_indexes,
									 std::vector<int> new_state_indexes,
									 SolutionNode* explore_node) override;
	void delete_potential_state(std::vector<int> potential_state_indexes,
								SolutionNode* explore_node) override;
	void clear_potential_state() override;

	void construct_fold_inputs(std::vector<int>& loop_scope_counts,
							   int& curr_index,
							   SolutionNode* explore_node) override;

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

	void activate_state_networks(Problem& problem,
								 double* state_vals,
								 bool* states_on,
								 bool backprop,
								 std::vector<NetworkHistory*>& network_historys);
	void activate_state_networks_with_potential(Problem& problem,
												double* state_vals,
												bool* states_on,
												double* potential_state_vals,
												std::vector<int>& potential_state_indexes,
												bool backprop,
												std::vector<NetworkHistory*>& network_historys);

	void backprop_state_networks(double* state_errors,
								 bool* states_on,
								 std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_errors_with_no_weight_change(
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_state_networks_with_potential(double* potential_state_errors,
												std::vector<int>& potential_state_indexes,
												std::vector<NetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_ACTION_H */