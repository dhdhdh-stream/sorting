#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "network.h"

const int NODE_TYPE_NORMAL = 0;
const int NODE_TYPE_IF_START = 1;
const int NODE_TYPE_IF_END = 2;
const int NODE_TYPE_LOOP_START = 3;
const int NODE_TYPE_LOOP_END = 4;

const int EXPLORE_STATE_EXPLORE = 0;
const int EXPLORE_STATE_LEARN = 1;
const int EXPLORE_STATE_MEASURE = 2;
// TODO: Add EXPLORE_STATE_COMPARE_MISGUESS

const int EXPLORE_TARGET_TYPE_GOOD = 0;
const int EXPLORE_TARGET_TYPE_BAD = 1;

const int EXPLORE_TYPE_RE_EVAL = -1;
const int EXPLORE_TYPE_NONE = 0;
const int EXPLORE_TYPE_EXPLORE = 1;
const int EXPLORE_TYPE_LEARN_PATH = 2;
const int EXPLORE_TYPE_LEARN_STATE = 3;
const int EXPLORE_TYPE_MEASURE = 4;

const int EXPLORE_NODE_STATE_NOT = -1;
const int EXPLORE_NODE_STATE_LEARN = 0;
const int EXPLORE_NODE_STATE_MEASURE = 1;

const int EXPLORE_DECISION_TYPE_JUMP = 0;
const int EXPLORE_DECISION_TYPE_NO_JUMP = 1;

const int EXPLORE_DECISION_TYPE_HALT = 0;
const int EXPLORE_DECISION_TYPE_NO_HALT = 1;

// TODO: Add folds

class SolutionNode {
public:
	Solution* solution;

	int node_index;
	int node_type;

	std::vector<int> score_network_inputs_state_indexes;
	std::vector<int> score_network_inputs_potential_state_indexes;
	Network* score_network;
	std::vector<int> score_network_backprop_indexes;
	// TODO: add certainty networks

	double average_unique_future_nodes;
	double average_score;

	int explore_state;
	int explore_iter_index;
	
	std::vector<Action> try_path;
	int explore_target_type;

	int explore_node_state;

	// SolutionNode(Solution* solution,
	// 			 int node_index);
	// SolutionNode(Solution* solution,
	// 			 int node_index,
	// 			 std::ifstream& save_file);
	virtual ~SolutionNode();

	// virtual void add_potential_state(int potential_state_index);

	virtual void reset() = 0;

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
								   std::vector<int>& explore_decisions) = 0;
	virtual void backprop(double score,
						  double misguess,
						  double* state_errors,
						  bool* states_on,
						  int& iter_explore_type,
						  SolutionNode* iter_explore_node,
						  double* potential_state_errors,
						  bool* potential_states_on,
						  std::vector<NetworkHistory*>& network_historys,
						  std::vector<int>& explore_decisions) = 0;

	virtual void explore_increment(double score) = 0;

	void increment_unique_future_nodes(int num_future_nodes);

	// void extend_with_potential();

	void save(std::ofstream& save_file);
	void save_for_display(std::ofstream& save_file);

protected:
	double activate_score_network(Problem& problem,
								  double* state_vals,
								  bool* states_on);
	double activate_score_network_eval(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   std::vector<NetworkHistory*>& network_historys);
	// activate_score_network_path identical to activate_score_network_eval
	double activate_score_network_state(Problem& problem,
										double* state_vals,
										bool* states_on,
										double* potential_state_vals,
										bool* potential_states_on,
										std::vector<NetworkHistory*>& network_historys);
	
	void backprop_score_network_eval(double score,
									 double* state_errors,
									 bool* states_on,
									 std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_path(double score,
									 double* state_errors,
									 bool* states_on,
									 std::vector<NetworkHistory*>& network_historys);
	void backprop_score_network_state(double score,
									  double* potential_state_errors,
									  vector<NetworkHistory*>& network_historys);
	// don't need potential_states_on because information in network_history
};

#endif /* SOLUTION_NODE_H */