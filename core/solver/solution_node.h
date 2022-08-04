#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "network.h"
#include "solver.h"

const int EXPLORE_STATE_EXPLORE = 0;
const int EXPLORE_STATE_MEASURE_AVERAGE = 1;
const int EXPLORE_STATE_LEARN_SCORES = 2;
const int EXPLORE_STATE_MEASURE_INFORMATION = 3;

class Solver;
class SolutionNode {
public:
	Solver* solver;
	int node_index;
	int path_length;	// assume fixed path_length (i.e., no branch merge) for now

	std::vector<int> children_indexes;
	std::vector<Action> children_actions;
	std::vector<Network*> children_score_networks;
	std::vector<std::string> children_score_network_names;
	std::vector<Network*> children_information_networks;
	std::vector<std::string> children_information_network_names;
	std::vector<int> children_counts;

	int count;
	double average_score;
	Network* score_network;
	std::string score_network_name;
	Network* certainty_network;
	std::string certainty_network_name;

	int explore_state;
	int current_explore_id;
	int candidate_iter;
	double best_information;
	std::vector<Action> best_candidate;
	std::vector<Action> current_candidate;

	// EXPLORE_STATE_MEASURE_AVERAGE
	int measure_average_p_index;
	double measure_average_average_score;

	// EXPLORE_STATE_LEARN_SCORES
	Network* learn_scores_network;

	// EXPLORE_STATE_MEASURE_INFORMATION
	int measure_information_p_index;
	double measure_information_think_good_and_good;
	double measure_information_think_good_but_bad;

	std::mutex children_mtx;
	std::mutex explore_mtx;

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
				 int& explore_status,
				 int& explore_id,
				 int& explore_candidate_iter);
	void update(std::vector<double> observations,
				int chosen_path,
				double score,
				double misguess);
	void update_explore(std::vector<double> observations,
						std::vector<double> full_observations,
						int explore_status,
						int explore_id,
						int explore_candidate_iter,
						double score);
	void update_explore_candidate(int explore_status,
								  int explore_id,
								  int explore_candidate_iter,
								  std::vector<Action> candidate);

	void save(std::ofstream& save_file);
	void save_for_display(std::ofstream& save_file);

private:
	void update_self(std::vector<double> observations,
					 double score);
};

#endif /* SOLUTION_NODE_H */