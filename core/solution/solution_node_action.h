#ifndef SOLUTION_NODE_ACTION_H
#define SOLUTION_NODE_ACTION_H

#include <vector>

#include "action.h"
#include "solution_node.h"

const int TEMP_NODE_STATE_NOT = -1;
const int TEMP_NODE_STATE_LEARN = 0;
const int TEMP_NODE_STATE_MEASURE = 1;

class SolutionNodeAction : public SolutionNode {
public:
	std::vector<int> score_network_inputs_state_indexes;
	std::vector<int> score_network_inputs_potential_state_indexes;
	Network* score_network;

	Action action;

	std::vector<std::vector<int>> state_network_inputs_state_indexes;
	std::vector<Network*> state_networks;
	std::vector<int> state_networks_target_states;

	std::vector<std::vector<int>> potential_inputs_state_indexes;
	std::vector<std::vector<int>> potential_potential_inputs_state_indexes;
	std::vector<Network*> potential_state_networks;
	std::vector<int> potential_state_networks_target_states;

	SolutionNode* next;

	SolutionNode* previous;

	int temp_node_state;

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

	double activate_score_network(Problem& problem,
								  double* state_vals,
								  bool* states_on,
								  bool backprop,
								  std::vector<NetworkHistory*>& network_historys);
	double activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		std::vector<int>& potential_state_indexes,
		bool backprop,
		std::vector<NetworkHistory*>& network_historys);
	
	void backprop_score_network(double score,
								double* state_errors,
								bool* states_on,
								std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_with_potential(
		double score,
		double* potential_state_errors,
		std::vector<NetworkHistory*>& network_historys);
	// don't need potential_states_indexes because information in network_history
};

#endif /* SOLUTION_NODE_ACTION_H */