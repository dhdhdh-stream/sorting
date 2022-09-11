#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "explore_step_history.h"
#include "fold_network.h"
#include "iter_explore.h"
#include "network.h"
#include "problem.h"
#include "re_eval_step_history.h"
#include "score_network.h"

const int NODE_TYPE_EMPTY = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_START_SCOPE = 2;
const int NODE_TYPE_JUMP_SCOPE = 3;
// const int NODE_TYPE_LOOP_SCOPE = 4;

const int SCOPE_LOCATION_TOP = 0;
const int SCOPE_LOCATION_BRANCH = 1;

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

class ExploreStepHistory;
class ReEvalStepHistory;
class SolutionNode {
public:
	int node_type;

	SolutionNode* next;

	SolutionNode* parent_scope;
	int scope_location;
	int scope_child_index;
	int scope_node_index;

	double node_weight;

	std::vector<int> local_state_sizes;
	ScoreNetwork* score_network;	// used mainly by action, but also empty
	double average_misguess;
	double explore_weight;

	int explore_state;
	int explore_iter_index;

	int parent_jump_scope_start_non_inclusive_index;
	int parent_jump_end_non_inclusive_index;

	std::vector<SolutionNode*> explore_path;

	int explore_existing_path_flat_size;
	int explore_new_path_flat_size;

	// may be NULL
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

	int explore_new_state_size;	// TODO: try different sizes/try 0
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

	virtual SolutionNode* re_eval(Problem& problem,
								  std::vector<std::vector<double>>& state_vals,
								  std::vector<SolutionNode*>& scopes,
								  std::vector<int>& scope_states,
								  std::vector<ReEvalStepHistory>& instance_history,
								  std::vector<AbstractNetworkHistory*>& network_historys) = 0;
	virtual void re_eval_backprop(double score,
								  std::vector<std::vector<double>>& state_errors,
								  std::vector<ReEvalStepHistory>& instance_history,
								  std::vector<AbstractNetworkHistory*>& network_historys) = 0;
	virtual SolutionNode* explore(Problem& problem,
								  std::vector<std::vector<double>>& state_vals,
								  std::vector<SolutionNode*>& scopes,
								  std::vector<int>& scope_states,
								  std::vector<int>& scope_locations,
								  IterExplore*& iter_explore,
								  std::vector<ExploreStepHistory>& instance_history,
								  std::vector<AbstractNetworkHistory*>& network_historys,
								  bool& abandon_instance) = 0;
	virtual void explore_backprop(double score,
								  std::vector<std::vector<double>>& state_errors,
								  IterExplore*& iter_explore,
								  std::vector<ExploreStepHistory>& instance_history,
								  std::vector<AbstractNetworkHistory*>& network_historys) = 0;
	virtual void explore_increment(double score,
								   IterExplore*& iter_explore) = 0;

	virtual void re_eval_increment() = 0;
	
	virtual SolutionNode* deep_copy(int inclusive_start_layer) = 0;
	virtual void set_is_temp_node(bool is_temp_node) = 0;
	virtual void initialize_local_state(std::vector<int>& explore_node_local_state_sizes) = 0;
	virtual void setup_flat(std::vector<int>& loop_scope_counts,
							int& curr_index,
							SolutionNode* explore_node) = 0;
	virtual void setup_new_state(SolutionNode* explore_node,
								 int new_state_size) = 0;
	// virtual void reset_state(SolutionNode* explore_node);
	virtual void get_min_misguess(double& min_misguess) = 0;	// need to include SolutionNodeEmpty
	virtual void cleanup_explore(SolutionNode* explore_node) = 0;
	virtual void collect_new_state_networks(SolutionNode* explore_node,
											std::vector<SolutionNode*>& existing_nodes,
											std::vector<Network*>& new_state_networks) = 0;
	virtual void insert_scope(int layer,
							  int new_state_size) = 0;
	virtual void reset_explore() = 0;

	virtual void save(std::vector<int>& scope_states,
					  std::vector<int>& scope_locations,
					  std::ofstream& save_file) = 0;
	virtual void save_for_display(std::ofstream& save_file) = 0;

	void explore_callback_helper(Problem& problem,
								 std::vector<std::vector<double>>& state_vals,
								 std::vector<SolutionNode*>& scopes,
								 std::vector<int>& scope_states,
								 std::vector<int>& scope_locations,
								 std::vector<ExploreStepHistory>& instance_history,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void is_explore_helper(std::vector<SolutionNode*>& scopes,
						   std::vector<int>& scope_states,
						   std::vector<int>& scope_locations,
						   IterExplore*& iter_explore,
						   bool& is_first_explore);
	SolutionNode* explore_helper(bool is_first_explore,
								 Problem& problem,
								 std::vector<SolutionNode*>& scopes,
								 std::vector<int>& scope_states,
								 std::vector<int>& scope_locations,
								 IterExplore*& iter_explore,
								 std::vector<ExploreStepHistory>& instance_history,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void explore_callback_backprop_helper(std::vector<std::vector<double>>& state_errors,
										  std::vector<ExploreStepHistory>& instance_history,
										  std::vector<AbstractNetworkHistory*>& network_historys);
	void explore_backprop_helper(double score,
								 std::vector<ExploreStepHistory>& instance_history,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void explore_increment_helper(double score,
								  IterExplore*& iter_explore);

	void explore_abandon_helper();

	void backprop_explore_jump_score_network(double score,
											 std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_explore_no_jump_score_network(double score,
												std::vector<AbstractNetworkHistory*>& network_historys);

	void state_backprop_explore_jump_score_network(double score,
												   std::vector<double>& new_state_errors,
												   std::vector<AbstractNetworkHistory*>& network_historys);
	void state_backprop_explore_no_jump_score_network(double score,
													  std::vector<double>& new_state_errors,
													  std::vector<AbstractNetworkHistory*>& network_historys);

	void full_backprop_explore_jump_score_network(double score,
												  std::vector<double>& new_state_errors,
												  std::vector<AbstractNetworkHistory*>& network_historys);
	void full_backprop_explore_no_jump_score_network(double score,
													 std::vector<double>& new_state_errors,
													 std::vector<AbstractNetworkHistory*>& network_historys);

	void backprop_explore_small_jump_score_network(double score,
												   std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_explore_small_no_jump_score_network(double score,
													  std::vector<AbstractNetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_H */