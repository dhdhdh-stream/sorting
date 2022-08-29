#ifndef SOLUTION_NODE_IF_START_H
#define SOLUTION_NODE_IF_START_H

#include "solution_node.h"
#include "solution_node_if_end.h"

class SolutionNodeIfEnd;
class SolutionNodeIfStart : public SolutionNode {
public:
	std::vector<int> children_networks_inputs_state_indexes;
	std::vector<int> children_networks_inputs_potential_state_indexes;
	std::vector<SolutionNode*> children_nodes;
	std::vector<Network*> children_score_networks;
	std::vector<Network*> children_certainty_networks;

	std::vector<bool> children_on;

	SolutionNodeIfEnd* end;

	SolutionNode* previous;

	int explore_child_index;

	SolutionNodeIfStart(SolutionNode* parent,
						int node_index);
	SolutionNodeIfStart(Solution* solution,
						int node_index,
						std::ifstream& save_file);
	~SolutionNodeIfStart();

	void reset() override;

	void add_potential_state(std::vector<int> potential_state_indexes,
							 SolutionNode* explore_node) override;
	void extend_with_potential_state(std::vector<int> potential_state_indexes,
									 std::vector<int> new_state_indexes,
									 SolutionNode* explore_node) override;
	void delete_potential_state(std::vector<int> potential_state_indexes,
								SolutionNode* explore_node) override;

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
				  std::vector<int>& potential_state_indexes,
				  std::vector<NetworkHistory*>& network_historys,
				  std::vector<int>& explore_decisions,
				  std::vector<bool>& explore_loop_decisions) override;

	void save(std::ofstream& save_file) override;
	void save_for_display(std::ofstream& save_file) override;

	void activate_children_networks(Problem& problem,
									double* state_vals,
									bool* states_on,
									bool backprop,
									std::vector<NetworkHistory*>& network_historys,
									double& best_score,
									int& best_index,
									bool save_for_display,
									std::ofstream& display_file);
	void activate_children_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		std::vector<int>& potential_state_indexes,
		bool backprop,
		std::vector<NetworkHistory*>& network_historys,
		double& best_score,
		int& best_index,
		bool save_for_display,
		std::ofstream& display_file);

	void backprop_children_networks(double score,
									double misguess,
									double* state_errors,
									bool* states_on,
									std::vector<NetworkHistory*>& network_historys);
	void backprop_children_networks_errors_with_no_weight_change(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_children_networks_with_potential(double score,
												   double misguess,
												   double* potential_state_errors,
												   std::vector<NetworkHistory*>& network_historys);
	// don't need potential_states_indexes because information in network_history
};

#endif /* SOLUTION_NODE_IF_START_H */