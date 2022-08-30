#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "network.h"
#include "problem.h"
#include "solution.h"

const int NODE_TYPE_START = 0;
const int NODE_TYPE_END = 1;
const int NODE_TYPE_ACTION = 2;
const int NODE_TYPE_IF_START = 3;
const int NODE_TYPE_IF_END = 4;
const int NODE_TYPE_LOOP_START = 5;
const int NODE_TYPE_LOOP_END = 6;

const int EXPLORE_TYPE_RE_EVAL = -1;
const int EXPLORE_TYPE_NONE = 0;
const int EXPLORE_TYPE_EXPLORE = 1;
const int EXPLORE_TYPE_LEARN_JUMP = 2;
const int EXPLORE_TYPE_MEASURE_JUMP = 3;
const int EXPLORE_TYPE_LEARN_LOOP = 4;
const int EXPLORE_TYPE_MEASURE_LOOP = 5;

const int EXPLORE_PATH_STATE_EXPLORE = 0;
const int EXPLORE_PATH_STATE_LEARN_JUMP = 1;
const int EXPLORE_PATH_STATE_MEASURE_JUMP = 2;
const int EXPLORE_PATH_STATE_LEARN_LOOP = 3;
const int EXPLORE_PATH_STATE_MEASURE_LOOP = 4;

const int EXPLORE_DECISION_TYPE_N_A = 0;
const int EXPLORE_DECISION_TYPE_EXPLORE = 1;
const int EXPLORE_DECISION_TYPE_NO_EXPLORE = 2;

// TODO: Add folds

class IterExplore;
class Solution;
class SolutionNode {
public:
	Solution* solution;

	int node_index;
	int node_type;

	double node_weight;

	int explore_path_state;
	int explore_path_iter_index;
	
	int explore_loop_iter;
	std::vector<SolutionNode*> explore_path;

	SolutionNode* explore_start_non_inclusive;
	SolutionNode* explore_start_inclusive;
	SolutionNode* explore_end_inclusive;
	SolutionNode* explore_end_non_inclusive;

	std::vector<int> explore_network_inputs_state_indexes;
	Network* explore_jump_network;
	Network* explore_no_jump_network;
	std::vector<int> explore_loop_states;
	Network* explore_halt_network;
	Network* explore_no_halt_network;

	int explore_path_measure_count;
	int explore_explore_is_good;
	int explore_explore_is_bad;
	double explore_explore_misguess;
	int explore_no_explore_is_good;
	int explore_no_explore_is_bad;
	double explore_no_explore_misguess;
	// TODO: compare misguess

	bool node_is_on;

	virtual ~SolutionNode();

	virtual void reset() = 0;

	virtual void add_potential_state(std::vector<int> potential_state_indexes,
									 SolutionNode* explore_node) = 0;
	virtual void extend_with_potential_state(std::vector<int> potential_state_indexes,
											 std::vector<int> new_state_indexes,
											 SolutionNode* explore_node) = 0;
	virtual void delete_potential_state(std::vector<int> potential_state_indexes,
										SolutionNode* explore_node) = 0;
	virtual void clear_potential_state() = 0;

	// virtual SolutionNode* activate(Problem& problem,
	// 							   double* state_vals,
	// 							   bool* states_on,
	// 							   std::vector<SolutionNode*>& loop_scopes,
	// 							   std::vector<int>& loop_scope_counts,
	// 							   int& iter_explore_type,
	// 							   SolutionNode*& iter_explore_node,
	// 							   IterExplore*& iter_explore,
	// 							   double* potential_state_vals,
	// 							   std::vector<int>& potential_state_indexes,
	// 							   std::vector<NetworkHistory*>& network_historys,
	// 							   std::vector<std::vector<double>>& guesses,
	// 							   std::vector<int>& explore_decisions,
	// 							   std::vector<bool>& explore_loop_decisions,
	// 							   bool save_for_display,
	// 							   std::ofstream& display_file) = 0;
	virtual SolutionNode* activate(Problem& problem,
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
								   std::vector<bool>& explore_loop_decisions) = 0;
	virtual void backprop(double score,
						  double misguess,
						  double* state_errors,
						  bool* states_on,
						  int& iter_explore_type,
						  SolutionNode*& iter_explore_node,
						  double* potential_state_errors,
						  std::vector<int>& potential_state_indexes,
						  std::vector<NetworkHistory*>& network_historys,
						  std::vector<int>& explore_decisions,
						  std::vector<bool>& explore_loop_decisions) = 0;

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

protected:
	double activate_explore_jump_network(Problem& problem,
										 double* state_vals,
										 bool* states_on,
										 bool backprop,
										 std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_jump_network(double score,
									   double* state_errors,
									   bool* states_on,
									   std::vector<NetworkHistory*>& network_historys);

	double activate_explore_no_jump_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											bool backprop,
											std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_no_jump_network(double score,
										  double* state_errors,
										  bool* states_on,
										  std::vector<NetworkHistory*>& network_historys);

	double activate_explore_halt_network(Problem& problem,
										 double* state_vals,
										 bool* states_on,
										 double* potential_state_vals,
										 bool backprop,
										 std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_halt_network(double score,
									   double* potential_state_errors,
									   std::vector<NetworkHistory*>& network_historys);

	double activate_explore_no_halt_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											double* potential_state_vals,
											bool backprop,
											std::vector<NetworkHistory*>& network_historys);
	void backprop_explore_no_halt_network(double score,
										  double* potential_state_errors,
										  std::vector<NetworkHistory*>& network_historys);
};

const int ITER_EXPLORE_TYPE_JUMP = 0;
const int ITER_EXPLORE_TYPE_LOOP = 1;

class IterExplore {
public:
	int iter_explore_type;
	
	std::vector<Action> try_path;

	SolutionNode* iter_start_non_inclusive;
	SolutionNode* iter_start_inclusive;
	SolutionNode* iter_end_inclusive;
	SolutionNode* iter_end_non_inclusive;

	std::vector<int> available_state;

	int iter_child_index;

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