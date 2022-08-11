#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "explore.h"
#include "network.h"
#include "solver.h"

class Explore;
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

	double average_score;

	Explore* explore;

	std::mutex children_mtx;

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