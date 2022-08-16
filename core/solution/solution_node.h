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

const int CANDIDATE_STATE_EXPLORE = 0;
const int CANDIDATE_STATE_MEASURE_AVERAGE = 1;
const int CANDIDATE_STATE_LEARN_SCORES = 2;
const int CANDIDATE_STATE_MEASURE_INFORMATION = 3;
// assume no loops for now

const int STATE_EXPLORE_TYPE_GREEDY = 0;
const int STATE_EXPLORE_TYPE_FOLD = 1;
// no folds for now

class SolutionNode {
public:
	Solution* solution;

	int node_index;
	int node_type;

	// explore gotos, but don't have them directly

	std::vector<int> score_network_inputs_state_indexes;
	Network* score_network;
	std::vector<int> score_network_backprop_indexes;

	double average_future_nodes;

	double average;
	double misguess;

	Candidate* candidate;

	SolutionNode(Solver* solver,
				 int node_index,
				 int path_length);
	SolutionNode(Solver* solver,
				 int node_index,
				 std::ifstream& save_file);
	~SolutionNode();

	void add_child(int child_index,
				   Action child_action);

	void process(std::vector<double> observations,
				 int& chosen_path,
				 std::vector<double>& guesses,
				 bool& force_eval,
				 bool save_for_display,
				 std::ofstream& display_file);
	
	void update_average(double score);
	void update_score_network(std::vector<double> observations,
							  int chosen_path,
							  double score);
	void update_information_network(std::vector<double> observations,
									int chosen_path,
									double misguess);

	void save(std::ofstream& save_file);
	void save_for_display(std::ofstream& save_file);

	virtual void add_potential_state(int potential_state_index);

	virtual void reset() = 0;

	virtual SolutionNode* tune(Problem& problem,
							   double* state_vals,
							   bool* states_on,
							   std::vector<NetworkHistory*>& network_historys) = 0;
	virtual void tune_update(double score,
							 double* state_errors,
							 bool* states_on,
							 std::vector<NetworkHistory*>& network_historys) = 0;

	virtual void increment() = 0;

	void extend_with_potential();

protected:
	void tune_score_network(Problem& problem,
							double* state_vals,
							bool* states_on,
							std::vector<NetworkHistory*>& network_historys);
	void tune_update_score_network(double score,
								   double* state_errors,
								   bool* states_on,
								   std::vector<NetworkHistory*>& network_historys);
	void increment_score_network();
};

#endif /* SOLUTION_NODE_H */