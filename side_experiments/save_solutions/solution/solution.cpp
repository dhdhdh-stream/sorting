#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "helpers.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

/**
 * - simply merge every fixed number of timesteps
 *   - hopefully prevents solution from getting too stale and thrashing
 */
const int RUN_TIMESTEPS = 30;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_val_average = original->curr_val_average;
	this->curr_val_standard_deviation = original->curr_val_standard_deviation;

	for (map<int, Scope*>::iterator it = original->scopes.begin();
			it != original->scopes.end(); it++) {
		Scope* scope = new Scope();
		scope->id = it->first;
		this->scopes[scope->id] = scope;
	}

	for (map<int, Scope*>::iterator it = original->scopes.begin();
			it != original->scopes.end(); it++) {
		this->scopes[it->first]->copy_from(original->scopes[it->first],
										   this);
	}

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link(this);
	}

	clean();
}

Solution::~Solution() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		delete it->second;
	}

	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void Solution::load(ifstream& input_file) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_val_average_line;
	getline(input_file, curr_val_average_line);
	this->curr_val_average = stod(curr_val_average_line);

	string curr_val_standard_deviation_line;
	getline(input_file, curr_val_standard_deviation_line);
	this->curr_val_standard_deviation = stod(curr_val_standard_deviation_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		Scope* scope = new Scope();
		scope->id = scope_id;
		this->scopes[scope_id] = scope;
	}

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		this->scopes[scope_id]->load(input_file,
									 this);
	}

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link(this);
	}

	clean();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::clean_inputs(Scope* scope,
							int node_id) {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->clean_inputs(scope,
								 node_id);
	}
}

void Solution::replace_obs_node(Scope* scope,
								int original_node_id,
								int new_node_id) {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->replace_obs_node(scope,
									 original_node_id,
									 new_node_id);
	}
}

void Solution::clean() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->clean();
	}
}

void Solution::measure_update() {
	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		update_counts(this->existing_scope_histories[h_index],
					  h_index);
	}

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_val_histories.size(); h_index++) {
		sum_vals += this->existing_target_val_histories[h_index];
	}
	double new_score = sum_vals / (double)this->existing_target_val_histories.size();

	cout << "new_score: " << new_score << endl;

	if (this->timestamp >= RUN_TIMESTEPS) {
		this->timestamp = -1;
	}

	this->curr_val_average = new_score;

	double sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_val_histories.size(); h_index++) {
		sum_variance += (this->existing_target_val_histories[h_index] - this->curr_val_average)
			* (this->existing_target_val_histories[h_index] - this->curr_val_average);
	}
	this->curr_val_standard_deviation = sqrt(sum_variance / (double)this->existing_target_val_histories.size());

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->measure_update((int)this->existing_scope_histories.size());
	}
}

void Solution::clean_scopes() {
	map<int, Scope*>::iterator it = this->scopes.begin();
	while (it != this->scopes.end()) {
		bool still_used = false;
		for (map<int, Scope*>::iterator i_it = this->scopes.begin();
				i_it != this->scopes.end(); i_it++) {
			if (it->first != i_it->first) {
				for (map<int, AbstractNode*>::iterator node_it = i_it->second->nodes.begin();
						node_it != i_it->second->nodes.end(); node_it++) {
					switch (node_it->second->type) {
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)node_it->second;
							if (scope_node->scope == it->second) {
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
			for (map<int, Scope*>::iterator i_it = this->scopes.begin();
					i_it != this->scopes.end(); i_it++) {
				i_it->second->clean_inputs(it->second);

				for (int c_index = 0; c_index < (int)i_it->second->child_scopes.size(); c_index++) {
					if (i_it->second->child_scopes[c_index] == it->second) {
						i_it->second->child_scopes.erase(i_it->second->child_scopes.begin() + c_index);
						break;
					}
				}
			}

			delete it->second;
			it = this->scopes.erase(it);
		} else {
			it++;
		}
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->timestamp << endl;
	output_file << this->curr_val_average << endl;
	output_file << this->curr_val_standard_deviation << endl;

	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
	}

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
		it->second->save(output_file);
	}
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
		it->second->save_for_display(output_file);
	}
}
