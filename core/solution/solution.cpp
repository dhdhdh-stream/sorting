#include "solution.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <set>

#include "candidate_branch.h"
#include "candidate_replace.h"
#include "candidate_start_branch.h"
#include "candidate_start_replace.h"
#include "definitions.h"
#include "jump_scope.h"
#include "solution_node_action.h"
#include "utilities.h"

using namespace std;

Solution::Solution() {
	this->id = time(NULL);

	this->start_scope = new StartScope();

	this->average_score = 0.5;

	action_dictionary = new ActionDictionary();
}

Solution::Solution(ifstream& save_file) {
	string id_line;
	getline(save_file, id_line);
	this->id = stol(id_line);

	vector<int> scope_states;
	vector<int> scope_locations;
	this->start_scope = new StartScope(scope_states,
									   scope_locations,
									   save_file);

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);

	action_dictionary->load(save_file);
}

Solution::~Solution() {
	delete this->start_scope;

	delete action_dictionary;
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

		set<SolutionNode*> unique_nodes_visited;

		SolutionNode* curr_node = this->start_scope;
		while (true) {
			SolutionNode* next_node = curr_node->re_eval(problem,
														 state_vals,
														 scopes,
														 scope_states,
														 instance_history,
														 network_historys);

			if (unique_nodes_visited.find(curr_node) == unique_nodes_visited.end()) {
				unique_nodes_visited.insert(curr_node);
			}

			if (next_node == NULL) {
				break;
			}
			curr_node = next_node;
		}

		double score = problem.score_result();
		this->average_score = 0.9999*this->average_score + 0.0001*score;

		this->start_scope->re_eval_increment();
		for (set<SolutionNode*>::iterator it = unique_nodes_visited.begin(); it != unique_nodes_visited.end(); it++) {
			SolutionNode* node = *it;
			node->node_weight += 0.0001;
		}

		double initial_misguess = abs(score - this->average_score);
		double best_misguess = initial_misguess;
		for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
			if (instance_history[h_index].node_visited->node_type == NODE_TYPE_ACTION
					|| (instance_history[h_index].node_visited->node_type == NODE_TYPE_START_SCOPE
					&& instance_history[h_index].scope_state == START_SCOPE_STATE_ENTER)) {
				double misguess = abs(score - instance_history[h_index].guess);
				if (misguess < best_misguess) {
					best_misguess = misguess;
				}
			}
		}

		double info_gain = best_misguess - initial_misguess;
		if (info_gain > 0.0) {
			vector<double> weights;
			vector<int> weight_divides;
			double curr_best_misguess = initial_misguess;
			for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
				if (instance_history[h_index].node_visited->node_type == NODE_TYPE_ACTION
						|| (instance_history[h_index].node_visited->node_type == NODE_TYPE_START_SCOPE
						&& instance_history[h_index].scope_state == START_SCOPE_STATE_ENTER)) {
					double misguess = abs(score - instance_history[h_index].guess);
					if (misguess > curr_best_misguess) {
						weights.push_back((misguess - curr_best_misguess)/info_gain);
						curr_best_misguess = misguess;
					} else {
						weights.push_back(0.0);
					}
					weight_divides.push_back(1);
				} else if (instance_history[h_index].node_visited->node_type == NODE_TYPE_JUMP_SCOPE) {
					if (instance_history[h_index].scope_state == JUMP_SCOPE_STATE_IF
							|| instance_history[h_index].scope_state == JUMP_SCOPE_STATE_EXIT) {
						weight_divides.back()++;
					}
				}
				// SolutionNodeEmpty cannot explore
			}

			int weights_index = -1;
			for (int h_index = 0; h_index < (int)instance_history.size(); h_index++) {
				if (instance_history[h_index].node_visited->node_type == NODE_TYPE_ACTION
						|| (instance_history[h_index].node_visited->node_type == NODE_TYPE_START_SCOPE
						&& instance_history[h_index].scope_state == START_SCOPE_STATE_ENTER)) {
					weights_index++;
					double weight_update = weights[weights_index]/weight_divides[weights_index];
					instance_history[h_index].node_visited->explore_weight = 0.9999*instance_history[h_index].node_visited->explore_weight
						+ 0.0001*weight_update;
				} else if (instance_history[h_index].node_visited->node_type == NODE_TYPE_JUMP_SCOPE) {
					if (instance_history[h_index].scope_state == JUMP_SCOPE_STATE_IF) {
						// TODO: add if explore
					} else if (instance_history[h_index].scope_state == JUMP_SCOPE_STATE_EXIT) {
						double weight_update = weights[weights_index]/weight_divides[weights_index];
						instance_history[h_index].node_visited->explore_weight = 0.9999*instance_history[h_index].node_visited->explore_weight
							+ 0.0001*weight_update;
					}
				}
				// SolutionNodeEmpty cannot explore
			}
		}

		vector<vector<double>> state_errors;
		while (instance_history.size() > 0) {
			instance_history.back().node_visited->re_eval_backprop(
				score,
				state_errors,
				instance_history,
				network_historys);
		}
	} else {
		// explore
		vector<vector<double>> state_vals;
		vector<SolutionNode*> scopes;
		vector<int> scope_states;
		vector<int> scope_locations;
		IterExplore* iter_explore = NULL;
		vector<ExploreStepHistory> instance_history;
		vector<AbstractNetworkHistory*> network_historys;
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
			// TODO: cleanup
		}

		double score = problem.score_result();

		vector<vector<double>> state_errors;
		while (instance_history.size() > 0) {
			instance_history.back().node_visited->explore_backprop(score,
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
			int best_index = -1;
			double best_score_increase = 0.0;
			double best_info_gain = 0.0;
			for (int c_index = 0; c_index < (int)this->candidates.size(); c_index++) {
				if (this->candidates[c_index]->type == CANDIDATE_BRANCH) {
					CandidateBranch* candidate_branch = (CandidateBranch*)this->candidates[c_index];
					double effective_score_increase = candidate_branch->explore_node->node_weight
													  *candidate_branch->branch_percent
													  *candidate_branch->score_increase;
					if (effective_score_increase > best_score_increase) {
						best_score_increase = effective_score_increase;
						best_index = c_index;
					}
				} else if (this->candidates[c_index]->type == CANDIDATE_REPLACE) {
					CandidateReplace* candidate_replace = (CandidateReplace*)this->candidates[c_index];
					if (candidate_replace->replace_type == EXPLORE_REPLACE_TYPE_SCORE) {
						double effective_score_increase = candidate_replace->explore_node->node_weight
														  *candidate_replace->score_increase;
						if (effective_score_increase > best_score_increase) {
							best_score_increase = effective_score_increase;
							best_index = c_index;
						}
					} else {
						// candidate_replace->replace_type == EXPLORE_REPLACE_TYPE_INFO
						if (best_score_increase == 0.0) {
							if (candidate_replace->info_gain > best_info_gain) {
								best_info_gain = candidate_replace->info_gain;
								best_index = c_index;
							}
						}
					}
				} else if (this->candidates[c_index]->type == CANDIDATE_START_BRANCH) {
					CandidateStartBranch* candidate_start_branch = (CandidateStartBranch*)this->candidates[c_index];
					double effective_score_increase = candidate_start_branch->branch_percent
													  *candidate_start_branch->score_increase;
					if (effective_score_increase > best_score_increase) {
						best_score_increase = effective_score_increase;
						best_index = c_index;
					}
				} else if (this->candidates[c_index]->type == CANDIDATE_START_REPLACE) {
					CandidateStartReplace* candidate_start_replace = (CandidateStartReplace*)this->candidates[c_index];
					if (candidate_start_replace->replace_type == EXPLORE_REPLACE_TYPE_SCORE) {
						double effective_score_increase = candidate_start_replace->score_increase;
						if (effective_score_increase > best_score_increase) {
							best_score_increase = effective_score_increase;
							best_index = c_index;
						}
					} else {
						// candidate_replace->replace_type == EXPLORE_REPLACE_TYPE_INFO
						if (best_score_increase == 0.0) {
							if (candidate_start_replace->info_gain > best_info_gain) {
								best_info_gain = candidate_start_replace->info_gain;
								best_index = c_index;
							}
						}
					}
				}
			}

			this->start_scope->reset_explore();

			this->candidates[best_index]->apply();
			delete this->candidates[best_index];
			this->candidates.erase(this->candidates.begin()+best_index);

			for (int c_index = 0; c_index < (int)this->candidates.size(); c_index++) {
				this->candidates[c_index]->clean();
				delete this->candidates[c_index];
			}
			this->candidates.clear();
		}
	}
}

void Solution::save(ofstream& save_file) {
	save_file << this->id << endl;

	vector<int> scope_states;
	vector<int> scope_locations;
	this->start_scope->save(scope_states,
							scope_locations,
							save_file);

	save_file << this->average_score << endl;

	action_dictionary->save(save_file);
}
