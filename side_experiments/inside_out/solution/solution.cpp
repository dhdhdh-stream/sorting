#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const int MAX_STAGNANT_TIMESTEPS = 5;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}

	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = 0.0;

	this->best_timestamp = 0;
	this->best_score = 0.0;

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
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	clean();
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

	string best_timestamp_line;
	getline(input_file, best_timestamp_line);
	this->best_timestamp = stoi(best_timestamp_line);

	string best_score_line;
	getline(input_file, best_score_line);
	this->best_score = stod(best_score_line);

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

	input_file.close();

	clean();
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

void Solution::clean() {
	/**
	 * - remove duplicate ObsNodes
	 * 
	 * - clean for all scopes as ObsNodes could have been added during experiments
	 */
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		while (true) {
			bool removed_node = false;

			for (map<int, AbstractNode*>::iterator it = this->scopes[s_index]->nodes.begin();
					it != this->scopes[s_index]->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_OBS) {
					ObsNode* curr_obs_node = (ObsNode*)it->second;
					if (curr_obs_node->next_node != NULL
							&& curr_obs_node->next_node->type == NODE_TYPE_OBS
							&& curr_obs_node->next_node->ancestor_ids.size() == 1) {
						ObsNode* next_obs_node = (ObsNode*)curr_obs_node->next_node;

						for (int f_index = 0; f_index < (int)next_obs_node->factors.size(); f_index++) {
							Factor* new_factor = new Factor(next_obs_node->factors[f_index],
															this);
							curr_obs_node->factors.push_back(new_factor);

							replace_factor(this->scopes[s_index],
										   next_obs_node->id,
										   f_index,
										   curr_obs_node->id,
										   curr_obs_node->factors.size()-1);
						}

						replace_obs_node(this->scopes[s_index],
										 next_obs_node->id,
										 curr_obs_node->id);

						if (next_obs_node->next_node != NULL) {
							for (int a_index = 0; a_index < (int)next_obs_node->next_node->ancestor_ids.size(); a_index++) {
								if (next_obs_node->next_node->ancestor_ids[a_index] == next_obs_node->id) {
									next_obs_node->next_node->ancestor_ids.erase(
										next_obs_node->next_node->ancestor_ids.begin() + a_index);
									break;
								}
							}
							next_obs_node->next_node->ancestor_ids.push_back(curr_obs_node->id);
						}
						curr_obs_node->next_node_id = next_obs_node->next_node_id;
						curr_obs_node->next_node = next_obs_node->next_node;

						clean_inputs(this->scopes[s_index],
									 next_obs_node->id);

						this->scopes[s_index]->nodes.erase(next_obs_node->id);
						delete next_obs_node;

						removed_node = true;
						break;
					}
				}
			}

			if (!removed_node) {
				break;
			}
		}
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean();
	}

	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}
	this->existing_scope_histories.clear();
	this->existing_target_val_histories.clear();
}

void Solution::measure_update() {
	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		update_scores(this->existing_scope_histories[h_index],
					  this->existing_target_val_histories[h_index],
					  h_index);
	}

	double sum_score = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_val_histories.size(); h_index++) {
		sum_score += this->existing_target_val_histories[h_index];
	}
	double new_score = sum_score / (double)this->existing_target_val_histories.size();

	cout << "new_score: " << new_score << endl;

	if (new_score > this->best_score) {
		this->best_score = new_score;
		this->best_timestamp = this->timestamp;
	} else if (this->timestamp >= this->best_timestamp + MAX_STAGNANT_TIMESTEPS) {
		this->timestamp = -1;
	}

	this->curr_score = new_score;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->measure_update((int)this->existing_scope_histories.size());
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

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->best_timestamp << endl;
	output_file << this->best_score << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
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
