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

	vector<SolutionNode*> loop_scopes;
	vector<int> loop_scope_counts;

	double potential_state_vals[this->current_potential_state_counter] = {};
	bool potential_states_on[this->current_potential_state_counter] = {};

	// nodes_visited and network_historys should tie 1-to-1 (except for state_networks)
	vector<NetworkHistory*> network_historys;
	vector<SolutionNode*> nodes_visited;
	vector<double> guesses;	// also used to save explore diffs

	// only update on EXPLORE_TYPE_RE_EVAL
	set<SolutionNode*> unique_nodes_visited;
	vector<SolutionNode*> unique_nodes_visited_list;

	int iter_explore_type = EXPLORE_TYPE_NONE;
	if (rand()%10 == 0) {
		iter_explore_type = EXPLORE_TYPE_RE_EVAL;
	}
	SolutionNode* iter_explore_node = NULL;

	vector<int> explore_decisions;
	vector<double> explore_diffs;

	SolutionNode* curr_node = this->nodes[0];
	while (true) {
		bool is_first_time;
		if (unique_nodes_visited.find(curr_node) != unique_nodes_visited.end()) {
			is_first_time = false;
		} else {
			is_first_time = true;
		}

		SolutionNode* next_node = curr_node->activate(problem,
													  state_vals,
													  states_on,
													  loop_scopes,
													  loop_scope_counts,
													  is_first_time,
													  iter_explore_type,
													  iter_explore_node,
													  potential_state_vals,
													  potential_states_on,
													  network_historys,
													  guesses,
													  explore_decisions,
													  explore_diffs);

		if (iter_explore_type != EXPLORE_TYPE_NONE) {
			nodes_visited.push_back(curr_node);
		}

		if (is_first_time) {
			unique_nodes_visited.insert(curr_node);
			unique_nodes_visited_list.push_back(curr_node);
		}

		if (curr_node->node_index == 1) {
			break;
		}
		curr_node = next_node;
	}

	double score = problem.score_result();

	double sum_misguess = 0.0;
	double state_errors[this->current_state_counter] = {};
	double potential_state_errors[this->current_potential_state_counter] = {};
	for (int v_index = (int)nodes_visited.size()-1; v_index >= 0; v_index--) {
		sum_misguess += abs(score - guesses[v_index]);
		double misguess = sum_misguess/(nodes_visited.size() - v_index);

		backprop_nodes[v_index]->backprop(score,
										  misguess,
										  state_errors,
										  states_on,
										  iter_explore_type,
										  iter_explore_node,
										  potential_state_errors,
										  potential_states_on,
										  network_historys,
										  explore_decisions,
										  explore_diffs);
	}

	for (int u_index = 0; u_index < (int)unique_nodes_visited_list.size(); u_index++) {
		unique_nodes_visited_list[u_index]->increment_unique_future_nodes(
			(int)unique_nodes_visited_list.size() - u_index);
	}

	if (explore_node != NULL) {
		explore_node->explore_increment(score,
										iter_explore_type);
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
