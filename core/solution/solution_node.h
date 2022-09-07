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
const int NODE_TYPE_EMPTY = 1;
const int NODE_TYPE_START_SCOPE = 2;
const int NODE_TYPE_JUMP_SCOPE = 3;
// const int NODE_TYPE_LOOP_SCOPE = 4;

const int SCOPE_LOCATION_TOP;
const int SCOPE_LOCATION_BRANCH;

const int EXPLORE_STATE_EXPLORE = 0;
const int EXPLORE_STATE_LEARN_FLAT = 1;
const int EXPLORE_STATE_MEASURE_FLAT = 2;
const int EXPLORE_STATE_LEARN_FOLD_BRANCH = 3;
const int EXPLORE_STATE_LEARN_SMALL_BRANCH = 4;
const int EXPLORE_STATE_MEASURE_FOLD_BRANCH = 5;
const int EXPLORE_STATE_LEARN_FOLD_REPLACE = 6;
const int EXPLORE_STATE_LEARN_SMALL_REPLACE = 7;
const int EXPLORE_STATE_MEASURE_FOLD_REPLACE = 8;

const int EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE = 0;
const int EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE = 1;
const int EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE = 2;
const int EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE = 3;

const int EXPLORE_DECISION_TYPE_FOLD_N_A = 0;
const int EXPLORE_DECISION_TYPE_FOLD_EXPLORE = 1;
const int EXPLORE_DECISION_TYPE_FOLD_NO_EXPLORE = 2;

const int EXPLORE_REPLACE_TYPE_SCORE = 0;
const int EXPLORE_REPLACE_TYPE_INFO = 1;

class IterExplore;
class Solution;
class SolutionNode {
public:
	int node_type;

	SolutionNode* next;

	SolutionNode* parent_scope;
	int scope_location;
	int scope_child_index;
	int scope_node_index;
	std::vector<int> local_state;

	// percentage of instances that this node takes part in
	double node_weight;

	Network* score_network;
	// mainly used by solution_node_action, but also by empty to compare
	// if all else the same, shorter path goes
	double explore_weight;
	double average_misguess;

	int explore_state;
	int explore_iter_index;

	int parent_jump_scope_start_non_inclusive_index;
	int parent_jump_end_non_inclusive_index;

	std::vector<SolutionNode*> explore_path;

	int explore_existing_path_flat_size;
	int explore_new_path_flat_size;

	std::vector<FoldNetwork*> explore_state_networks;

	FoldNetwork* explore_jump_score_network;
	FoldNetwork* explore_no_jump_score_network;	

	// FoldNetwork* explore_score_improvement_network;
	// FoldNetwork* explore_score_prediction_if_exit_network;
	// FoldNetwork* explore_prediction_improvement_network;

	int explore_explore_explore_count;
	double explore_explore_explore_score;
	int explore_explore_no_explore_count;
	double explore_explore_no_explore_score;
	int explore_no_explore_explore_count;
	double explore_no_explore_explore_score;
	int explore_no_explore_no_explore_count;
	double explore_no_explore_no_explore_score;

	int explore_replace_type;
	double explore_replace_info_gain;

	int explore_new_state_size;	// TODO: try different sizes, try 0
	int explore_existing_path_fold_input_index_on_inclusive;
	int explore_new_path_fold_input_index_on_inclusive;

	Network* explore_small_jump_score_network;
	Network* explore_small_no_jump_score_network;

	int explore_fold_explore_count;
	double explore_fold_explore_score;
	int explore_fold_no_explore_count;
	double explore_fold_no_explore_score;

	double explore_fold_replace_score;

	bool is_temp_node;

	virtual ~SolutionNode();	// needs to be recursive

	// all recursive
	virtual SolutionNode* deep_copy(int inclusive_start_layer) = 0;
	virtual void initialize_local_scope(std::vector<int>& local_state) = 0;
	// also initializes networks
	virtual void setup_flat(std::vector<int>& loop_scope_counts,
							int& curr_index,
							SolutionNode* explore_node) = 0;
	virtual void setup_new_state(SolutionNode* explore_node,
								 int new_state_size);
	// virtual void reset_state(SolutionNode* explore_node);
	virtual void insert_scope(int layer) = 0;
	virtual void get_min_misguess();
	virtual void cleanup_explore(SolutionNode* explore_node);
	virtual void collect_new_state_networks(SolutionNode* explore_node,
											std::vector<SolutionNode*>& existing_nodes,
											std::vector<Network*>& new_state_networks);
	// also cleans up explores

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

#endif /* SOLUTION_NODE_H */