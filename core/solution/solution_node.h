#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "network.h"
#include "problem.h"
#include "solution.h"

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_EMPTY = 1;	// only appear by themselves, and can only set state
const int NODE_TYPE_START_SCOPE = 2;
const int NODE_TYPE_JUMP_SCOPE = 3;
// const int NODE_TYPE_LOOP_SCOPE = 3;

const int EXPLORE_STATE_EXPLORE = 0;
const int EXPLORE_STATE_LEARN_JUMP_FLAT = 1;
const int EXPLORE_STATE_MEASURE_JUMP_FLAT = 2;
const int EXPLORE_STATE_LEARN_JUMP_FOLD = 1;
const int EXPLORE_STATE_MEASURE_JUMP_FOLD = 2;
// const int EXPLORE_STATE_PRE_EVAL_LOOP = 3;
// const int EXPLORE_STATE_LEARN_LOOP_FLAT = 4;
// const int EXPLORE_STATE_MEASURE_LOOP_FLAT = 5;
// const int EXPLORE_STATE_LEARN_LOOP_FOLD = 6;
// const int EXPLORE_STATE_MEASURE_LOOP_FOLD = 7;
// don't include loops for now

const int EXPLORE_DECISION_TYPE_N_A = 0;
const int EXPLORE_DECISION_TYPE_EXPLORE = 1;
const int EXPLORE_DECISION_TYPE_NO_EXPLORE = 2;

class IterExplore;
class Solution;
class SolutionNode {
public:
	Solution* solution;

	SolutionNode* parent_scope;

	int node_index;
	int node_type;

	SolutionNode* previous;
	SolutionNode* next;

	double explore_weight;

	// only used for node weights (no longer part of any decision making)
	// uses all state available
	// not accurate
	// train greedily
	Network* score_network;

	int explore_state;
	int explore_iter_index;

	SolutionNode* explore_scope_start_non_inclusive;
	SolutionNode* explore_scope_start_inclusive;
	SolutionNode* explore_branch_start_non_inclusive;
	SolutionNode* explore_branch_start_inclusive;
	SolutionNode* explore_scope_end_inclusive;
	SolutionNode* explore_scope_end_non_inclusive;

	int explore_flat_size;
	int explore_fold_input_index_on;

	// may be compound actions
	std::vector<SolutionNode*> explore_jump_potential_nodes;
	FlatNetwork* explore_jump_score_network;
	FlatNetwork* explore_no_jump_score_network;
	Network* explore_jump_full_score_network;
	Network* explore_no_jump_full_score_network;
	int explore_path_flat_size;
	std::vector<FlatNetwork*> explore_state_networks;
	
	FlatNetwork* explore_score_improvement_network;
	FlatNetwork* explore_score_prediction_if_exit_network;
	FlatNetwork* explore_prediction_improvement_network;
	// for loops, for outer state, just try to optimize greedily?

	int explore_flat_misguess;	// used to compare the fold against to see if more state needs to be added
	// try 1-5? after 5, if it still sucks, but better than existing branch, then take

	int explore_explore_measure_count;
	int explore_explore_is_good;
	int explore_explore_is_bad;
	double explore_explore_misguess;
	int explore_no_explore_measure_count;
	int explore_no_explore_is_good;
	int explore_no_explore_is_bad;
	double explore_no_explore_misguess;
	// TODO: for jumps, need for each branch
	// only declare victory on clearly good results
	// or keep list of results, and choose best after a while
	// if keep list of results, then don't even need nested weights anymore
	// yeah, no nested weights, just free-for-all

	std::map<SolutionNode*, FoldHelper*> fold_helpers;

	bool node_is_on;

	virtual ~SolutionNode();

	virtual void setup_fold(std::vector<int>& loop_scope_counts,
						   int& curr_index,
						   SolutionNode* explore_node) = 0;

	virtual SolutionNode* re_eval();
	virtual SolutionNode* explore(Problem& problem,
								   double* state_vals,
								   bool* states_on,
								   std::vector<SolutionNode*>& loop_scopes,
								   std::vector<int>& loop_scope_counts,
								   std::vector<bool>& loop_decisions,
								   int& iter_explore_type,
								   SolutionNode*& iter_explore_node,
								   IterExplore*& iter_explore,
								   std::vector<NetworkHistory*>& network_historys,
								   std::vector<std::vector<double>>& guesses,
								   std::vector<int>& explore_decisions,
								   std::vector<SolutionNode*>& nodes_visited,
								   std::vector<double>& observations,
								   bool& cancel_run,
								   bool save_for_display,
								   std::ofstream& display_file) = 0;
	virtual void backprop(double score,
						  double misguess,
						  double* state_errors,
						  bool* states_on,
						  std::vector<bool>& loop_decisions,
						  int& iter_explore_type,
						  SolutionNode*& iter_explore_node,
						  double* potential_state_errors,
						  std::vector<int>& potential_state_indexes,
						  std::vector<NetworkHistory*>& network_historys,
						  std::vector<int>& explore_decisions) = 0;

	void activate_helper(Problem& problem,
						 double* state_vals,
						 bool* states_on,
						 int& iter_explore_type,
						 SolutionNode*& iter_explore_node,
						 double* potential_state_vals,
						 std::vector<int>& potential_state_indexes,
						 std::vector<NetworkHistory*>& network_historys,
						 std::vector<std::vector<double>>& guesses);
	void backprop_helper(double score,
						 double misguess,
						 double* state_errors,
						 bool* states_on,
						 int& iter_explore_type,
						 SolutionNode*& iter_explore_node,
						 double* potential_state_errors,
						 std::vector<NetworkHistory*>& network_historys);

	SolutionNode* explore_activate(Problem& problem,
								   double* state_vals,
								   bool* states_on,
								   std::vector<SolutionNode*>& loop_scopes,
								   std::vector<int>& loop_scope_counts,
								   int& iter_explore_type,
								   SolutionNode*& iter_explore_node,
								   IterExplore*& iter_explore,
								   bool is_first_explore,
								   double* potential_state_vals,
								   std::vector<int>& potential_state_indexes,
								   std::vector<NetworkHistory*>& network_historys,
								   std::vector<std::vector<double>>& guesses,
								   std::vector<int>& explore_decisions);
	void explore_backprop(double score,
						  double misguess,
						  double* state_errors,
						  bool* states_on,
						  SolutionNode*& iter_explore_node,
						  double* potential_state_errors,
						  std::vector<NetworkHistory*>& network_historys,
						  std::vector<int>& explore_decisions);
	void explore_increment(double score,
						   IterExplore* iter_explore);
	void clear_explore();

	void update_node_weight(double new_node_weight);

	virtual void save(std::ofstream& save_file) = 0;
	virtual void save_for_display(std::ofstream& save_file) = 0;

	void score_network_add_potential_state(std::vector<int> potential_state_indexes);
	void score_network_extend_with_potential_state(std::vector<int> potential_state_indexes,
												   std::vector<int> new_state_indexes);
	void score_network_delete_potential_state(std::vector<int> potential_state_indexes);
	void score_network_clear_potential_state();

	void load_score_network(std::ifstream& save_file);
	void save_score_network(std::ofstream& save_file);

	void activate_score_network(Problem& problem,
								double* state_vals,
								bool* states_on,
								bool backprop,
								std::vector<NetworkHistory*>& network_historys,
								double& predicted_score,
								double& predicted_misguess);
	void activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		std::vector<int>& potential_state_indexes,
		bool backprop,
		std::vector<NetworkHistory*>& network_historys,
		double& predicted_score,
		double& predicted_misguess);
	
	void backprop_score_network(double score,
								double misguess,
								double* state_errors,
								bool* states_on,
								std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_errors_with_no_weight_change(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_with_potential(
		double score,
		double misguess,
		double* potential_state_errors,
		std::vector<NetworkHistory*>& network_historys);
	// don't need potential_states_indexes because information in network_history

	void activate_explore_jump_network(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   bool backprop,
									   std::vector<NetworkHistory*>& network_historys,
									   double& predicted_score,
									   double& predicted_misguess);
	void backprop_explore_jump_network(double score,
									   double misguess,
									   double* state_errors,
									   bool* states_on,
									   std::vector<NetworkHistory*>& network_historys);

	void activate_explore_halt_network(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   double* potential_state_vals,
									   bool backprop,
									   std::vector<NetworkHistory*>& network_historys,
									   double& predicted_score,
									   double& predicted_misguess);
	void backprop_explore_halt_network(double score,
									   double misguess,
									   double* potential_state_errors,
									   std::vector<NetworkHistory*>& network_historys);

	void activate_explore_no_halt_network(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  double* potential_state_vals,
										  bool backprop,
										  std::vector<NetworkHistory*>& network_historys,
										  double& predicted_score,
										  double& predicted_misguess);
	void backprop_explore_no_halt_network(double score,
										  double misguess,
										  double* potential_state_errors,
										  std::vector<NetworkHistory*>& network_historys);
};

// const int ITER_EXPLORE_TYPE_JUMP = 0;
// const int ITER_EXPLORE_TYPE_LOOP = 1;

const int ITER_EXPLORE_TYPE_JUMP = 0;
const int ITER_EXPLORE_TYPE_APPEND = 1;
const int ITER_EXPLORE_TYPE_BRANCH_START = 2;

class IterExplore {
public:
	std::vector<SolutionNode*> explore_node;
	int iter_explore_type;
	
	std::vector<Action> try_path;

	SolutionNode* iter_start_non_inclusive;
	SolutionNode* iter_start_inclusive;
	SolutionNode* iter_end_inclusive;
	SolutionNode* iter_end_non_inclusive;

	std::vector<int> available_state;

	int iter_child_index;

	std::vector<double> full_guesses;

	IterExplore(int iter_explore_type,
				std::vector<Action> try_path,
				SolutionNode* iter_start_non_inclusive,
				SolutionNode* iter_start_inclusive,
				SolutionNode* iter_end_inclusive,
				SolutionNode* iter_end_non_inclusive,
				std::vector<int> available_state,
				int iter_child_index) {
		this->iter_explore_type = iter_explore_type;
		this->try_path = try_path;
		this->iter_start_non_inclusive = iter_start_non_inclusive;
		this->iter_start_inclusive = iter_start_inclusive;
		this->iter_end_inclusive = iter_end_inclusive;
		this->iter_end_non_inclusive = iter_end_non_inclusive;
		this->available_state = available_state;
		this->iter_child_index = iter_child_index;
	};
	~IterExplore() {
		// do nothing
	}
};

#endif /* SOLUTION_NODE_H */