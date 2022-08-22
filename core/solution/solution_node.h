#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "network.h"

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_IF_START = 1;
const int NODE_TYPE_IF_END = 2;
const int NODE_TYPE_LOOP_START = 3;
const int NODE_TYPE_LOOP_END = 4;

const int EXPLORE_TYPE_RE_EVAL = -1;
const int EXPLORE_TYPE_NONE = 0;
const int EXPLORE_TYPE_EXPLORE = 1;
const int EXPLORE_TYPE_LEARN_PATH = 2;
const int EXPLORE_TYPE_LEARN_STATE = 3;
const int EXPLORE_TYPE_MEASURE_PATH = 4;
const int EXPLORE_TYPE_MEASURE_STATE = 5;

const int EXPLORE_PATH_STATE_EXPLORE = 0;
const int EXPLORE_PATH_STATE_LEARN = 1;
const int EXPLORE_PATH_STATE_MEASURE = 2;

const int EXPLORE_STATE_STATE_LEARN = 0;
const int EXPLORE_STATE_STATE_MEASURE = 1;

const int PATH_EXPLORE_TYPE_JUMP = 0;
const int PATH_EXPLORE_TYPE_LOOP = 1;

const int PATH_TARGET_TYPE_GOOD = 0;
const int PATH_TARGET_TYPE_BAD = 1;

const int EXPLORE_DECISION_TYPE_N_A = 0;
const int EXPLORE_DECISION_TYPE_EXPLORE = 1;
const int EXPLORE_DECISION_TYPE_NO_EXPLORE = 2;

const int TEMP_NODE_STATE_NOT = -1;
const int TEMP_NODE_STATE_LEARN = 0;
const int TEMP_NODE_STATE_MEASURE = 1;

// TODO: Add folds

class SolutionNode {
public:
	Solution* solution;

	int node_index;
	int node_type;

	std::vector<int> network_inputs_state_indexes;
	std::vector<int> network_inputs_potential_state_indexes;
	
	Network* score_network;

	// TODO: add certainty networks

	double average_unique_future_nodes;
	double average_score;
	double average_misguess;

	std::vector<int> scope_potential_states;

	int explore_path_state;
	int explore_path_iter_index;
	int explore_state_state;
	int explore_state_iter_index;

	int path_explore_type;
	int path_target_type;
	std::vector<Action> try_path;
	std::vector<SolutionNode*> explore_path;
	
	SolutionNode* explore_start_non_inclusive;
	SolutionNode* explore_start_inclusive;
	SolutionNode* explore_end_inclusive;
	SolutionNode* explore_end_non_inclusive;
	
	Network* explore_if_network;
	Network* explore_halt_network;
	Network* explore_no_halt_network;
	
	int explore_path_measure_count;
	double explore_explore_is_good;
	double explore_explore_is_bad;
	double explore_no_explore_is_good;
	double explore_no_explore_is_bad;
	// TODO: compare misguess

	int explore_state_measure_count;
	std::vector<double> explore_state_scores;
	std::vector<double> explore_state_misguesses;

	bool has_explored_state; // without folds, explore states once

	int temp_node_state;

	virtual ~SolutionNode() {};

	virtual void reset() = 0;

	virtual void add_potential_state(std::vector<int> potential_state_indexes,
									 SolutionNode* scope);
	virtual void extend_with_potential_state(std::vector<int> potential_state_indexes,
											 std::vector<int> new_state_indexes,
											 SolutionNode* scope);
	virtual void reset_potential_state(std::vector<int> potential_state_indexes,
									   SolutionNode* scope);

	virtual SolutionNode* activate(Problem& problem,
								   double* state_vals,
								   bool* states_on,
								   std::vector<SolutionNode*>& loop_scopes,
								   std::vector<int>& loop_scope_counts,
								   bool is_first_time,
								   int& iter_explore_type,
								   SolutionNode* iter_explore_node,
								   double* potential_state_vals,
								   bool* potential_states_on
								   std::vector<NetworkHistory*>& network_historys,
								   std::vector<double>& guesses,
								   std::vector<int>& explore_decisions,
								   std::vector<double>& explore_diffs,
								   std::vector<bool>& explore_loop_decisions) = 0;
	virtual void backprop(double score,
						  double misguess,
						  double* state_errors,
						  bool* states_on,
						  int& iter_explore_type,
						  SolutionNode* iter_explore_node,
						  double* potential_state_errors,
						  bool* potential_states_on,
						  std::vector<NetworkHistory*>& network_historys,
						  std::vector<int>& explore_decisions
						  std::vector<double>& explore_diffs,
						  std::vector<bool>& explore_loop_decisions) = 0;

	virtual void clear_potential_state();

	void activate_score_network_helper(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   int& iter_explore_type,
									   SolutionNode* iter_explore_node,
									   double* potential_state_errors,
									   bool* potential_states_on,
									   std::vector<NetworkHistory*>& network_historys,
									   std::vector<double>& guesses);
	void explore(double score,
				 Problem& problem,
				 double* state_vals,
				 bool* states_on,
				 std::vector<SolutionNode*>& loop_scopes,
				 std::vector<int>& loop_scope_counts,
				 int& iter_explore_type,
				 SolutionNode* iter_explore_node,
				 double* potential_state_errors,
				 bool* potential_states_on,
				 std::vector<NetworkHistory*>& network_historys,
				 std::vector<double>& guesses,
				 std::vector<int>& explore_decisions,
				 std::vector<double>& explore_diffs);
	void backprop_explore_and_score_network_helper(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		int& iter_explore_type,
		SolutionNode* iter_explore_node,
		double* potential_state_errors,
		bool* potential_states_on,
		std::vector<NetworkHistory*>& network_historys,
		std::vector<int>& explore_decisions);
	void explore_increment(double score,
						   int iter_explore_type);
	void clear_explore();

	void increment_unique_future_nodes(int num_future_nodes);

	// virtual void save(std::ofstream& save_file) = 0;
	// virtual void save_for_display(std::ofstream& save_file) = 0;

protected:
	void add_potential_state_for_score_network(std::vector<int> potential_state_indexes);
	void extend_state_for_score_network(std::vector<int> potential_state_indexes,
										std::vector<int> new_state_indexes);
	void reset_potential_state_for_score_network(std::vector<int> potential_state_indexes);
	void clear_potential_state_for_score_network();

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
		bool* potential_states_on,
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
		vector<NetworkHistory*>& network_historys);
	// don't need potential_states_on because information in network_history

	double activate_explore_if_network(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   bool backprop,
									   std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_if_network(double score,
									 double* state_errors,
									 bool* states_on,
									 std::vector<NetworkHistory*>& network_historys);

	double activate_explore_halt_network(Problem& problem,
										 double* state_vals,
										 bool* states_on,
										 bool backprop,
										 std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_halt_network(double score,
									   double* state_errors,
									   bool* states_on,
									   std::vector<NetworkHistory*>& network_historys);

	double activate_explore_no_halt_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											bool backprop,
											std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_no_halt_network(double score,
										  double* states_errors,
										  bool* states_on,
										  std::vector<NetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_H */