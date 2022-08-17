#include "solver.h"

#include <iostream>
#include <fstream>
#include <limits>

#include "definitions.h"
#include "network.h"
#include "utilities.h"

using namespace std;

Solver::Solver(Explore* explore) {
	this->explore = explore;

	SolutionNode* root_node = new SolutionNodeLoopStart(this, 0);
	this->nodes.push_back(root_node);
	SolutionNode* halt_node = new SolutionNodeLoopEnd(this, 1);
	this->nodes.push_back(halt_node);

	this->current_state_counter = 0;

	// current_potential_state_counter reset to 0 every cycle
}

Solver::~Solver() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}
}

void Solution::iteration() {
	Problem problem;
	
	double state_vals[this->current_state_counter] = {};
	bool states_on[this->current_state_counter] = {};

	double potential_state_vals[this->current_potential_state_counter] = {};
	bool potential_states_on[this->current_potential_state_counter] = {};

	vector<SolutionNode*> nodes_visited;
	map<SolutionNode*, int> node_visited_counts;
	vector<NetworkHistory*> network_historys;

	SolutionNode* explore_node = NULL;
	int explore_type = EXPLORE_TYPE_NONE;

	SolutionNode* curr_node = this->nodes[0];
	while (true) {
		int visited_count;
		map<SolutionNode*,int>::iterator it = node_visited_counts.find(curr_node);
		if (it == node_visited_counts.end()) {
			visited_count = 0;
		} else {
			visited_count = it->second;
		}

		SolutionNode* next_node = curr_node->activate(problem,
													  state_vals,
													  states_on,
													  visited_count,
													  explore_node,
													  explore_type,
													  potential_state_vals,
													  potential_states_on,
													  network_historys);
		nodes_visited.push_back(curr_node);
		if (it == node_visited_counts.end()) {
			node_visited_counts[curr_node] = 1;
		} else {
			it->second++;
		}

		if (curr_node->node_index == 1) {
			break;
		}

		curr_node = next_node;
	}

	double score = problem.score_result();

	double potential_state_errors[this->current_potential_state_counter] = {};
	for (int v_index = (int)nodes_visited.size()-1; v_index >= 0; v_index--) {
		nodes_visited[v_index]->backprop(score,
										 explore_node,
										 explore_type,
										 potential_state_errors,
										 potential_states_on,
										 network_historys);
	}

	for (map<SolutionNode*,int>::iterator it = node_visited_counts.begin();
			it != node_visited_counts.end(); it++) {
		it->first->increment(explore_node,
							 explore_type,
							 potential_states_on);
	}
}

void Solver::save() {
	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);

	this->nodes_mtx.lock();
	int num_nodes = (int)this->nodes.size();
	this->nodes_mtx.unlock();

	save_file << num_nodes << endl;
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		// TODO: save node type first
		this->nodes[n_index]->save(save_file);
	}

	save_file.close();
}
