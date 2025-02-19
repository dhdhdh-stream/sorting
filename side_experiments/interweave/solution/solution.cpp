#include "solution.h"

#include "action_node.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const int INIT_NUM_STEPS = 20;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = -1.0;

	/**
	 * - even though scopes[0] will not be reused, still good to start with:
	 *   - if artificially add empty scopes, may be difficult to extend from
	 *     - and will then junk up explore
	 *   - new scopes will be created from the reusable portions anyways
	 */

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->node_counter = 0;
	this->scopes.insert(this->scopes.begin(), new_scope);

	ObsNode* starting_node = new ObsNode();
	starting_node->parent = new_scope;
	starting_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[starting_node->id] = starting_node;

	AbstractNode* prev_node = starting_node;
	uniform_int_distribution<int> type_distribution(0, 2);
	for (int s_index = 0; s_index < INIT_NUM_STEPS; s_index++) {
		int type = type_distribution(generator);
		if (type >= 1 && this->existing_scopes.size() > 0) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = new_scope;
			new_scope_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_scope_node->id] = new_scope_node;

			uniform_int_distribution<int> existing_scope_distribution(0, this->existing_scopes.size()-1);
			new_scope_node->scope = this->existing_scopes[existing_scope_distribution(generator)];

			ObsNode* obs_node = (ObsNode*)prev_node;
			obs_node->next_node_id = new_scope_node->id;
			obs_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(prev_node->id);

			prev_node = new_scope_node;
		} else {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = new_scope;
			new_action_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = problem_type->random_action();

			ObsNode* obs_node = (ObsNode*)prev_node;
			obs_node->next_node_id = new_action_node->id;
			obs_node->next_node = new_action_node;

			new_action_node->ancestor_ids.push_back(prev_node->id);

			prev_node = new_action_node;
		}

		ObsNode* new_obs_node = new ObsNode();
		new_obs_node->parent = new_scope;
		new_obs_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_obs_node->id] = new_obs_node;

		switch (prev_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)prev_node;
				action_node->next_node_id = new_obs_node->id;
				action_node->next_node = new_obs_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)prev_node;
				scope_node->next_node_id = new_obs_node->id;
				scope_node->next_node = new_obs_node;
			}
			break;
		}

		new_obs_node->ancestor_ids.push_back(prev_node->id);

		prev_node = new_obs_node;
	}

	ObsNode* obs_node = (ObsNode*)prev_node;
	obs_node->next_node_id = -1;
	obs_node->next_node = NULL;
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
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link();
	}

	string num_existing_scopes_line;
	getline(input_file, num_existing_scopes_line);
	int num_existing_scopes = stoi(num_existing_scopes_line);
	for (int c_index = 0; c_index < num_existing_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->existing_scopes.push_back(solution->scopes[stoi(scope_id_line)]);
	}

	input_file.close();
}

void Solution::clean_inputs(Scope* scope,
							int node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean_inputs(scope,
											node_id);
	}
}

void Solution::clean() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_experiments();
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean();
	}

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

			for (int e_index = 0; e_index < (int)this->existing_scopes.size(); e_index++) {
				if (this->existing_scopes[e_index] == this->scopes[s_index]) {
					this->existing_scopes.erase(this->existing_scopes.begin() + e_index);
					break;
				}
			}

			delete this->scopes[s_index];
			this->scopes.erase(this->scopes.begin() + s_index);
		}
	}

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->id = s_index;
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->unlink();
	}
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->relink();
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->existing_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->existing_scopes.size(); c_index++) {
		output_file << this->existing_scopes[c_index]->id << endl;
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
