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
const int NODE_TYPE_GOTO_START = 5;
const int NODE_TYPE_GOTO_END = 6;

const int CANDIDATE_STATE_EXPLORE = 0;
const int CANDIDATE_STATE_MEASURE_AVERAGE = 1;
const int CANDIDATE_STATE_LEARN_SCORES = 2;
const int CANDIDATE_STATE_MEASURE_INFORMATION = 3;
// assume no loops for now

class SolutionNode {
public:
	int node_index;
	int node_type;

	std::vector<int> goto_node_indexes;
	std::vector<int> goto_score_networks_inputs_state_indexes;
	std::vector<Network*> goto_score_networks;
	std::vector<std::string> goto_score_network_names;

	std::vector<bool> goto_on;

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
};

#endif /* SOLUTION_NODE_H */