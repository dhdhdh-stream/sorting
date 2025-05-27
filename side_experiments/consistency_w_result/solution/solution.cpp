#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double STARTING_TIME_PENALTY = 0.0005;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_score = original->curr_score;

	this->curr_true_score = original->curr_true_score;
	this->best_true_score = original->best_true_score;
	this->best_true_score_timestamp = original->best_true_score_timestamp;
	this->curr_time_penalty = original->curr_time_penalty;

	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	this->obs_average_vals = original->obs_average_vals;
	this->obs_variances = original->obs_variances;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = -1.0;

	this->curr_true_score = -1.0;
	this->best_true_score = -1.0;
	this->best_true_score_timestamp = 0;
	this->curr_time_penalty = STARTING_TIME_PENALTY;

	/**
	 * - even though scopes[0] will not be reused, still good to start with:
	 *   - if artificially add empty scopes, may be difficult to extend from
	 *     - and will then junk up explore
	 *   - new scopes will be created from the reusable portions anyways
	 */

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->node_counter = 0;
	this->scopes.push_back(new_scope);

	ObsNode* starting_noop_node = new ObsNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_noop_node->average_score = 0.5;
	starting_noop_node->average_instances_per_run = 1.0;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	this->obs_average_vals = vector<double>(problem_type->num_obs(), 0.0);
	this->obs_variances = vector<double>(problem_type->num_obs(), 1.0);
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + name);

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string curr_true_score_line;
	getline(input_file, curr_true_score_line);
	this->curr_true_score = stod(curr_true_score_line);

	string best_true_score_line;
	getline(input_file, best_true_score_line);
	this->best_true_score = stod(best_true_score_line);

	string best_true_score_timestamp_line;
	getline(input_file, best_true_score_timestamp_line);
	this->best_true_score_timestamp = stoi(best_true_score_timestamp_line);

	string curr_time_penalty_line;
	getline(input_file, curr_time_penalty_line);
	this->curr_time_penalty = stod(curr_time_penalty_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file,
									this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
		string average_val_line;
		getline(input_file, average_val_line);
		this->obs_average_vals.push_back(stod(average_val_line));

		string variance_line;
		getline(input_file, variance_line);
		this->obs_variances.push_back(stod(variance_line));
	}

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::clean_inputs(Scope* scope,
							int node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean_inputs(scope,
											node_id);
	}
}

void Solution::replace_factor(Scope* scope,
							  int original_node_id,
							  int original_factor_index,
							  int new_node_id,
							  int new_factor_index) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->replace_factor(scope,
											  original_node_id,
											  original_factor_index,
											  new_node_id,
											  new_factor_index);
	}
}

void Solution::replace_obs_node(Scope* scope,
								int original_node_id,
								int new_node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->replace_obs_node(scope,
												original_node_id,
												new_node_id);
	}
}

void Solution::replace_scope(Scope* original_scope,
							 Scope* new_scope,
							 int new_scope_node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->replace_scope(original_scope,
											 new_scope,
											 new_scope_node_id);
	}
}

void Solution::clean_scopes() {
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
			for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
				this->scopes[is_index]->clean_inputs(this->scopes[s_index]);

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

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->id = s_index;
	}
}

void Solution::clean() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean();
	}
}

void Solution::measure_update() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->measure_update();
	}
}

void Solution::measure_match_update() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->measure_match_update();
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->curr_true_score << endl;
	output_file << this->best_true_score << endl;
	output_file << this->best_true_score_timestamp << endl;
	output_file << this->curr_time_penalty << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
		output_file << this->obs_average_vals[o_index] << endl;
		output_file << this->obs_variances[o_index] << endl;
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
