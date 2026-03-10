#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

// temp
#include "experiment.h"

using namespace std;

Solution::Solution() {
	this->score_index = 0;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}

	for (int s_index = 0; s_index < (int)this->outer_scopes.size(); s_index++) {
		delete this->outer_scopes[s_index];
	}
}

void Solution::load(ifstream& input_file) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->is_outer = false;
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	string num_outer_scopes_line;
	getline(input_file, num_outer_scopes_line);
	int num_outer_scopes = stoi(num_outer_scopes_line);

	for (int s_index = 0; s_index < num_outer_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->is_outer = true;
		scope->id = s_index;
		this->outer_scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file,
									this);
	}

	for (int s_index = 0; s_index < (int)this->outer_scopes.size(); s_index++) {
		this->outer_scopes[s_index]->load(input_file,
										  this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	for (int s_index = 0; s_index < (int)this->outer_scopes.size(); s_index++) {
		this->outer_scopes[s_index]->link(this);
	}

	string num_outer_root_scope_ids_line;
	getline(input_file, num_outer_root_scope_ids_line);
	int num_outer_root_scope_ids = stoi(num_outer_root_scope_ids_line);
	for (int r_index = 0; r_index < num_outer_root_scope_ids; r_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->outer_root_scope_ids.push_back(stoi(scope_id_line));
	}

	string num_last_scores_line;
	getline(input_file, num_last_scores_line);
	int num_last_scores = stoi(num_last_scores_line);
	for (int e_index = 0; e_index < num_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->last_scores.push_back(stod(score_line));
	}

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));

		string change_line;
		getline(input_file, change_line);
		this->change_history.push_back(change_line);
	}
}

void Solution::clean_scopes() {
	while (true) {
		bool removed_scope = false;
		for (int s_index = (int)this->scopes.size()-1; s_index >= 1; s_index--) {
			bool still_used = false;
			for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
				if (s_index != is_index) {
					for (map<int, AbstractNode*>::iterator it = this->scopes[is_index]->nodes.begin();
							it != this->scopes[is_index]->nodes.end(); it++) {
						switch (it->second->type) {
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)it->second;
								if (scope_node->scope == this->scopes[s_index]) {
									still_used = true;
									break;
								}
							}
							break;
						}
					}
				}

				if (still_used) {
					break;
				}
			}

			if (!still_used) {
				removed_scope = true;

				for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
					for (int c_index = 0; c_index < (int)this->scopes[is_index]->child_scopes.size(); c_index++) {
						if (this->scopes[is_index]->child_scopes[c_index] == this->scopes[s_index]) {
							this->scopes[is_index]->child_scopes.erase(this->scopes[is_index]->child_scopes.begin() + c_index);
							break;
						}
					}
				}

				delete this->scopes[s_index];
				this->scopes.erase(this->scopes.begin() + s_index);
			}
		}

		if (!removed_scope) {
			break;
		}
	}

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->id = s_index;
	}
}

void Solution::merge_outer() {
	/**
	 * - clear experiments
	 */
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = this->scopes[s_index]->nodes.begin();
				it != this->scopes[s_index]->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->experiment != NULL) {
					delete obs_node->experiment;
					obs_node->experiment = NULL;
				}
			}
		}
	}

	for (int s_index = 0; s_index < (int)this->outer_scopes.size(); s_index++) {
		this->scopes.push_back(this->outer_scopes[s_index]);
	}
	this->outer_scopes.clear();
	this->outer_root_scope_ids.clear();

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->is_outer = false;
		this->scopes[s_index]->id = s_index;
	}

	clean_scopes();
}

void Solution::save(ofstream& output_file) {
	/**
	 * TODO: swap endl to '\n' to avoid flushing
	 */
	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->state << endl;

	output_file << this->scopes.size() << endl;
	output_file << this->outer_scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	for (int s_index = 0; s_index < (int)this->outer_scopes.size(); s_index++) {
		this->outer_scopes[s_index]->save(output_file);
	}

	output_file << this->outer_root_scope_ids.size() << endl;
	for (int r_index = 0; r_index < (int)this->outer_root_scope_ids.size(); r_index++) {
		output_file << this->outer_root_scope_ids[r_index] << endl;
	}

	output_file << this->last_scores.size() << endl;
	for (list<double>::iterator it = this->last_scores.begin();
			it != this->last_scores.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
	}
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}

// temp
void Solution::print_experiment_statuses() {
	int num_explore = 0;
	int num_train_new = 0;
	int num_ramp = 0;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = this->scopes[s_index]->nodes.begin();
				it != this->scopes[s_index]->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->experiment != NULL) {
					Experiment* experiment = (Experiment*)obs_node->experiment;
					switch (experiment->state) {
					case EXPERIMENT_STATE_EXPLORE:
						num_explore++;
						break;
					case EXPERIMENT_STATE_TRAIN_NEW:
						num_train_new++;
						break;
					case EXPERIMENT_STATE_RAMP:
					case EXPERIMENT_STATE_MEASURE:
						num_ramp++;
						break;
					}
				}
			}
		}
	}

	cout << "num_explore: " << num_explore << endl;
	cout << "num_train_new: " << num_train_new << endl;
	cout << "num_ramp: " << num_ramp << endl;
}
