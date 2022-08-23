#include "solution.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <set>

#include "definitions.h"
#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"
#include "utilities.h"

using namespace std;

Solution::Solution(Explore* explore) {
	this->explore = explore;

	SolutionNode* root_node = new SolutionNodeLoopStart(this);
	this->nodes.push_back(root_node);
	SolutionNode* halt_node = new SolutionNodeLoopEnd(this);
	this->nodes.push_back(halt_node);

	this->current_state_counter = 0;
}

Solution::Solution(Explore* explore,
				   ifstream& save_file) {
	string num_nodes_line;
	getline(save_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index =0; n_index < num_nodes; n_index++) {
		string node_type_line;
		getline(save_file, node_type_line);
		int node_type = stoi(node_type_line);

		SolutionNode* node;
		if (node_type == NODE_TYPE_ACTION) {
			node = new SolutionNodeAction(this,
										  n_index,
										  save_file);
		} else if (node_type == NODE_TYPE_IF_START) {
			node = new SolutionNodeIfStart(this,
										   n_index,
										   save_file);
		} else if (node_type == NODE_TYPE_IF_END) {
			node = new SolutionNodeIfEnd(this,
										 n_index,
										 save_file);
		} else if (node_type == NODE_TYPE_LOOP_START) {
			node = new SolutionNodeLoopStart(this,
											 n_index,
											 save_file);
		} else if (node_type == NODE_TYPE_LOOP_END) {
			node = new SolutionNodeLoopEnd(this,
										   n_index,
										   save_file);
		}
		this->nodes.push_back(node);
	}
}

Solution::~Solution() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}
}

void Solution::iteration(bool tune,
						 bool save_for_display) {
	Problem problem;
	
	double state_vals[this->current_state_counter] = {};
	bool states_on[this->current_state_counter] = {};

	vector<SolutionNode*> loop_scopes;
	vector<int> loop_scope_counts;

	double potential_state_vals[this->current_potential_state_counter] = {};
	bool potential_states_on[this->current_potential_state_counter] = {};

	vector<NetworkHistory*> network_historys;
	vector<SolutionNode*> nodes_visited;
	vector<double> guesses;

	set<SolutionNode*> unique_nodes_visited;
	vector<SolutionNode*> unique_nodes_visited_list;

	int iter_explore_type = EXPLORE_TYPE_NONE;
	if (tune) {
		iter_explore_type = EXPLORE_TYPE_RE_EVAL;
	}
	SolutionNode* iter_explore_node = NULL;

	vector<int> explore_decisions;
	vector<double> explore_diffs;
	vector<bool> explore_loop_decisions;

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
													  explore_diffs,
													  explore_loop_decisions);

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
	// ignore last halt node
	for (int v_index = (int)nodes_visited.size()-2; v_index >= 0; v_index--) {
		double misguess;
		if (iter_explore_type == EXPLORE_TYPE_EXPLORE
				|| iter_explore_type == EXPLORE_TYPE_LEARN_PATH
				|| iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
			misguess = 0.0;
		} else {
			sum_misguess += abs(score - guesses[v_index]);
			misguess = sum_misguess/((double)nodes_visited.size() - v_index);
		}

		nodes_visited[v_index]->backprop(score,
										 misguess,
										 state_errors,
										 states_on,
										 iter_explore_type,
										 iter_explore_node,
										 potential_state_errors,
										 potential_states_on,
										 network_historys,
										 explore_decisions,
										 explore_diffs,
										 explore_loop_decisions);
	}

	for (int u_index = 0; u_index < (int)unique_nodes_visited_list.size(); u_index++) {
		unique_nodes_visited_list[u_index]->increment_unique_future_nodes(
			(int)unique_nodes_visited_list.size() - u_index);
	}

	if (iter_explore_node != NULL) {
		iter_explore_node->explore_increment(score,
											 iter_explore_type);
	}
}

void Solution::save(ofstream& save_file) {
	this->nodes_mtx.lock();
	int num_nodes = (int)this->nodes.size();
	this->nodes_mtx.unlock();

	save_file << num_nodes << endl;
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		save_file << this->nodes[n_index]->node_type << endl;
		this->nodes[n_index]->save(save_file);
	}
}
