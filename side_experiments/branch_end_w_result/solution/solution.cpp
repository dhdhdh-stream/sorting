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
#include "start_node.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		delete this->external_scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = 0.0;

	/**
	 * - even though scopes[0] will not be reused, still good to start with:
	 *   - if artificially add empty scopes, may be difficult to extend from
	 *     - and will then junk up explore
	 *   - new scopes will be created from the reusable portions anyways
	 */

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->is_external = false;
	new_scope->node_counter = 0;
	this->scopes.push_back(new_scope);

	StartNode* start_node = new StartNode();
	start_node->parent = new_scope;
	start_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[start_node->id] = start_node;

	ObsNode* end_node = new ObsNode();
	end_node->parent = new_scope;
	end_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[end_node->id] = end_node;

	start_node->next_node_id = end_node->id;
	start_node->next_node = end_node;

	end_node->ancestor_ids.push_back(start_node->id);

	end_node->next_node_id = -1;
	end_node->next_node = NULL;

	this->last_new_scope = NULL;
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

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		scope->is_external = false;
		this->scopes.push_back(scope);
	}

	string num_external_scopes_line;
	getline(input_file, num_external_scopes_line);
	int num_external_scopes = stoi(num_external_scopes_line);

	for (int s_index = 0; s_index < num_external_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		scope->is_external = true;
		this->external_scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file,
									this);
	}

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->load(input_file,
											 this);

		string is_root_line;
		getline(input_file, is_root_line);
		this->external_is_root.push_back(stoi(is_root_line));
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->link(this);
	}

	string last_new_scope_id_line;
	getline(input_file, last_new_scope_id_line);
	int last_new_scope_id = stoi(last_new_scope_id_line);
	if (last_new_scope_id == -1) {
		this->last_new_scope = NULL;
		this->new_scope_iters = 0;
	} else {
		this->last_new_scope = this->scopes[last_new_scope_id];

		string new_scope_iters_line;
		getline(input_file, new_scope_iters_line);
		this->new_scope_iters = stoi(new_scope_iters_line);
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

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->clear_verify();
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

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->clean_inputs(scope,
													 node_id);
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

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->replace_obs_node(
			scope,
			original_node_id,
			new_node_id);
	}
}

void Solution::clean_scopes() {
	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->scopes.push_back(this->external_scopes[s_index]);
	}
	this->external_scopes.clear();
	// temp
	this->external_is_root.clear();

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
					this->scopes[is_index]->clean_inputs(this->scopes[s_index]);

					for (set<Scope*>::iterator it = this->scopes[is_index]->child_scopes.begin();
							it != this->scopes[is_index]->child_scopes.end(); it++) {
						if (*it == this->scopes[s_index]) {
							this->scopes[is_index]->child_scopes.erase(*it);
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
		this->scopes[s_index]->is_external = false;
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->scopes.size() << endl;

	output_file << this->external_scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	for (int s_index = 0; s_index < (int)this->external_scopes.size(); s_index++) {
		this->external_scopes[s_index]->save(output_file);
		// temp
		output_file << this->external_is_root[s_index] << endl;
	}

	if (this->last_new_scope == NULL) {
		output_file << -1 << endl;
	} else {
		output_file << this->last_new_scope->id << endl;
		output_file << this->new_scope_iters << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
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
