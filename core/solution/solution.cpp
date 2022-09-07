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
	for (int n_index = 0; n_index < num_nodes; n_index++) {
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
	Problem problem;
	
	if (rand()%10 == 0) {
		// re-eval
		vector<vector<double>> state_vals;
		vector<SolutionNode*> scopes;
		vector<int> scope_states;
		vector<ReEvalStepHistory> instance_history;
		vector<AbstractNetworkHistory*> network_historys;

		SolutionNode* curr_node = this->start_scope;
		while (true) {
			SolutionNode* next_node = curr_node->re_eval(problem,
														 state_vals,
														 scopes,
														 scope_states,
														 instance_history,
														 network_historys);

			if (next_node == NULL) {
				break;
			}
			curr_node = next_node;
		}

		double score = problem.score_result();
		this->average_score = 0.9999*this->average_score + 0.0001*score;

		double initial_misguess = abs(score - this->average_score);
		double best_misguess = initial_misguess;
		for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
			if (instance_history[h_index]->node_visited->node_type == NODE_TYPE_ACTION
					|| (instance_history[h_index]->node_visited->node_type == NODE_TYPE_START_SCOPE
					&& instance_history[h_index]->scope_state == START_SCOPE_STATE_ENTER)) {
				double misguess = abs(score - instance_history[h_index]->guess);
				if (misguess < best_misguess) {
					best_misguess = misguess;
				}
			}
		}

		double info_gain = best_misguess - initial_misguess;
		if (info_gain > 0.0) {
			vector<double> information_gains;
			vector<int> information_gains_divides;
			double curr_best_misguess = initial_misguess;
			for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
				if (instance_history[h_index]->node_visited->node_type == NODE_TYPE_ACTION
						|| (instance_history[h_index]->node_visited->node_type == NODE_TYPE_START_SCOPE
						&& instance_history[h_index]->scope_state == START_SCOPE_STATE_ENTER)) {
					double misguess = abs(score - instance_history[h_index]->guess);
					if (misguess > curr_best_misguess) {
						information_gains.push_back(misguess - curr_best_misguess);
						curr_best_misguess = misguess;
					} else {
						information_gains.push_back(0.0);
					}
					information_gains_divides.push_back(1);
				} else if (instance_history[h_index]->node_visited->node_type == NODE_TYPE_JUMP_SCOPE) {
					if (instance_history[h_index]->scope_state == JUMP_SCOPE_STATE_IF
							|| instance_history[h_index]->scope_state == JUMP_SCOPE_STATE_EXIT) {
						information_gains_divides.back()++;
					}
				}
				// SolutionNodeEmpty cannot explore
			}

			int gains_index = -1;
			for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
				if (instance_history[h_index]->node_visited->node_type == NODE_TYPE_ACTION
						|| (instance_history[h_index]->node_visited->node_type == NODE_TYPE_START_SCOPE
						&& instance_history[h_index]->scope_state == START_SCOPE_STATE_ENTER)) {
					gains_index++;
					// update with information_gains[gains_index]/information_gains_divides[gains_index]
				} else if (instance_history[h_index]->node_visited->node_type == NODE_TYPE_JUMP_SCOPE) {
					if (instance_history[h_index]->scope_state == JUMP_SCOPE_STATE_IF
							|| instance_history[h_index]->scope_state == JUMP_SCOPE_STATE_EXIT) {
						// update with information_gains[gains_index]/information_gains_divides[gains_index]
					}
				}
				// SolutionNodeEmpty cannot explore
			}
		}

		// backprop
	} else {
		// explore
		vector<vector<double>> state_vals;
		vector<SolutionNode*> scopes;
		vector<int> scope_states;
		vector<int> scope_locations;
		IterExplore* iter_explore = NULL;
		vector<StepHistory> instance_history;
		vector<AbstractNetworkHistory*>& network_historys;
		bool abandon_instance = false;

		SolutionNode* curr_node = this->start_scope;
		while (true) {
			SolutionNode* next_node = curr_node->explore(problem,
														 state_vals,
														 scopes,
														 scope_states,
														 scope_locations,
														 iter_explore,
														 instance_history,
														 network_historys,
														 abandon_instance);

			if (abandon_instance) {
				break;
			}
			if (next_node == NULL) {
				break;
			}
			curr_node = next_node;
		}

		if (abandon_instance) {
			// cleanup
		}

		double score = problem.score_result();

		vector<vector<double>> state_errors;
		while (instance_history.size() > 0) {
			instance_history.back()->node_visited->explore_backprop(score,
																	state_errors,
																	iter_explore,
																	instance_history,
																	network_historys);
		}

		if (iter_explore != NULL) {
			iter_explore->explore_node->explore_increment(score,
														  iter_explore);
		}

		if (iter_explore != NULL) {
			delete iter_explore;
		}

		if (this->candidates.size() > 20) {

		}
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
