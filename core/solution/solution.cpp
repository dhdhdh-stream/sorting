#include "solution.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <set>

#include "definitions.h"
#include "solution_node_action.h"
#include "solution_node_end.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"
#include "solution_node_start.h"
#include "utilities.h"

using namespace std;

Solution::Solution(Explore* explore) {
	this->explore = explore;

	this->id = time(NULL);

	SolutionNode* start_node = new SolutionNodeStart(this);
	this->nodes.push_back(start_node);
	SolutionNode* end_node = new SolutionNodeEnd(this);
	this->nodes.push_back(end_node);

	this->current_state_counter = 2;	// start_node starts with 2

	this->average_score = 0.5;
}

Solution::Solution(Explore* explore,
				   ifstream& save_file) {
	this->explore = explore;

	string id_line;
	getline(save_file, id_line);
	this->id = stoi(id_line);

	string num_nodes_line;
	getline(save_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index =0; n_index < num_nodes; n_index++) {
		string node_type_line;
		getline(save_file, node_type_line);
		int node_type = stoi(node_type_line);

		SolutionNode* node;
		if (node_type == NODE_TYPE_START) {
			node = new SolutionNodeStart(this,
										 n_index,
										 save_file);
		} else if (node_type == NODE_TYPE_END) {
			node = new SolutionNodeEnd(this,
									   n_index,
									   save_file);
		} else if (node_type == NODE_TYPE_ACTION) {
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

	string current_state_counter_line;
	getline(save_file, current_state_counter_line);
	this->current_state_counter = stoi(current_state_counter_line);

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);
}

Solution::~Solution() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}
}

void Solution::iteration(bool tune,
						 bool save_for_display) {
	ofstream display_file;
	if (save_for_display) {
		display_file.open("../display.txt");
		display_file << this->nodes.size() << endl;
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->save_for_display(display_file);
		}
	}

	Problem problem;
	
	double state_vals[this->current_state_counter] = {};
	bool states_on[this->current_state_counter] = {};

	vector<SolutionNode*> loop_scopes;
	vector<int> loop_scope_counts;
	vector<bool> loop_decisions;

	double potential_state_vals[2] = {};
	vector<int> potential_state_indexes;

	vector<NetworkHistory*> network_historys;
	vector<SolutionNode*> nodes_visited;
	
	vector<vector<double>> guesses;
	vector<double> guess_segment;
	guess_segment.push_back(this->average_score);
	guesses.push_back(guess_segment);

	int iter_explore_type = EXPLORE_TYPE_NONE;
	if (tune) {
		iter_explore_type = EXPLORE_TYPE_RE_EVAL;
	}
	SolutionNode* iter_explore_node = NULL;
	IterExplore* iter_explore = NULL;

	vector<int> explore_decisions;

	vector<vector<double>> display_state_history;

	SolutionNode* curr_node = this->nodes[0];
	while (true) {
		SolutionNode* next_node = curr_node->activate(problem,
													  state_vals,
													  states_on,
													  loop_scopes,
													  loop_scope_counts,
													  loop_decisions,
													  iter_explore_type,
													  iter_explore_node,
													  iter_explore,
													  potential_state_vals,
													  potential_state_indexes,
													  network_historys,
													  guesses,
													  explore_decisions,
													  save_for_display,
													  display_file);

		if (save_for_display) {
			if (curr_node->node_type == NODE_TYPE_ACTION) {
				vector<double> state_snapshot;
				for (int s_index = 0; s_index < this->current_state_counter; s_index++) {
					state_snapshot.push_back(state_vals[s_index]);
				}
				display_state_history.push_back(state_snapshot);
			}
		}

		if (curr_node->node_type == NODE_TYPE_END) {
			break;
		}
		if (iter_explore_type != EXPLORE_TYPE_NONE) {
			nodes_visited.push_back(curr_node);
		}
		curr_node = next_node;
	}

	double score = problem.score_result();
	this->average_score = 0.9999*this->average_score + 0.0001*score;

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		double sum_segments = 0.0;
		for (int s_index = 0; s_index < (int)guesses.size(); s_index++) {
			double initial_guess = min(max(guesses[s_index][0], 0.0), 1.0);
			double initial_misguess = abs(score - initial_guess);
			double final_guess = min(max(guesses[s_index][guesses[s_index].size()-1], 0.0), 1.0);
			double final_misguess = abs(score - final_guess);
			sum_segments += initial_misguess - final_misguess;
		}

		vector<double> information_gains;
		vector<int> information_gains_divides;

		int guess_segment_index = 0;
		int guess_index = 1;
		double previous_misguess = abs(score - guesses[0][0]);
		for (int v_index = 0; v_index < (int)nodes_visited.size(); v_index++) {
			if (nodes_visited[v_index]->node_type == NODE_TYPE_START
					|| nodes_visited[v_index]->node_type == NODE_TYPE_ACTION) {
				double guess = min(max(guesses[guess_segment_index][guess_index], 0.0), 1.0);
				double current_misguess = abs(score - guess);

				double information_gain = previous_misguess - current_misguess;

				information_gains.push_back(information_gain);

				previous_misguess = current_misguess;

				guess_index++;

				information_gains_divides.push_back(1);
			} else if (nodes_visited[v_index]->node_type == NODE_TYPE_IF_START) {
				guess_segment_index++;
				guess_index = 1;

				double guess = min(max(guesses[guess_segment_index][0], 0.0), 1.0);
				previous_misguess = abs(score - guess);

				information_gains_divides.back()++;
			} else {
				information_gains_divides.back()++;
			}
		}

		int v_index = 0;
		for (int g_index = 0; g_index < (int)information_gains.size(); g_index++) {
			for (int d_index = 0; d_index < information_gains_divides[g_index]; d_index++) {
				nodes_visited[v_index]->update_node_weight(
					(information_gains[g_index]/information_gains_divides[g_index])/sum_segments);
				v_index++;
			}
		}
	}

	double sum_misguess = abs(score - guesses[0][0]);
	int misguess_count = 1;
	for (int s_index = 0; s_index < (int)guesses.size(); s_index++) {
		for (int g_index = 1; g_index < (int)guesses[s_index].size(); g_index++) {
			double guess = min(max(guesses[s_index][g_index], 0.0), 1.0);
			sum_misguess += abs(score - guess);

			misguess_count++;
		}
	}
	double misguess = sum_misguess/(double)misguess_count;

	double state_errors[this->current_state_counter] = {};
	double potential_state_errors[this->current_potential_state_counter] = {};
	for (int v_index = (int)nodes_visited.size()-1; v_index >= 0; v_index--) {
		nodes_visited[v_index]->backprop(score,
										 misguess,
										 state_errors,
										 states_on,
										 loop_decisions,
										 iter_explore_type,
										 iter_explore_node,
										 potential_state_errors,
										 potential_state_indexes,
										 network_historys,
										 explore_decisions);
	}

	if (save_for_display) {
		display_file << problem.initial_world.size() << endl;
		for (int i = 0; i < (int)problem.initial_world.size(); i++) {
			display_file << problem.initial_world[i] << endl;
		}

		if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP
				|| iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
			display_file << "explore" << endl;
			display_file << iter_explore_node->node_index << endl;
			display_file << iter_explore_node->explore_end_non_inclusive->node_index << endl;
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP
				|| iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
			display_file << "explore" << endl;
			display_file << iter_explore_node->node_index << endl;
			display_file << iter_explore_node->explore_start_inclusive->node_index << endl;
		} else {
			display_file << "no_explore" << endl;
		}

		display_file.close();
	}

	if (iter_explore_node != NULL) {
		iter_explore_node->explore_increment(score,
											 iter_explore);
	}

	if (iter_explore != NULL) {
		delete iter_explore;
	}
}

void Solution::save(ofstream& save_file) {
	save_file << this->id << endl;

	this->nodes_mtx.lock();
	int num_nodes = (int)this->nodes.size();
	this->nodes_mtx.unlock();

	save_file << num_nodes << endl;
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		save_file << this->nodes[n_index]->node_type << endl;
		this->nodes[n_index]->save(save_file);
	}

	save_file << this->current_state_counter << endl;

	save_file << this->average_score << endl;
}
