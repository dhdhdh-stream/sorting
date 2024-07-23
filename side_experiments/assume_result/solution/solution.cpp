#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double TRIM_PERCENTAGE = 0.2;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->average_score = original->average_score;

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

	this->max_num_actions = original->max_num_actions;
	this->num_actions_limit = original->num_actions_limit;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->average_score = -1.0;

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

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	starting_noop_node->action = Action(ACTION_NOOP, 0);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_noop_node->average_instances_per_run = 1.0;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	this->max_num_actions = 1;
	this->num_actions_limit = 40;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

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

	string max_num_actions_line;
	getline(input_file, max_num_actions_line);
	this->max_num_actions = stoi(max_num_actions_line);

	#if defined(MDEBUG) && MDEBUG
	this->num_actions_limit = 2*this->max_num_actions + 10;
	#else
	this->num_actions_limit = 10*this->max_num_actions + 10;
	#endif /* MDEBUG */

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

void Solution::clean() {
	for (int s_index = (int)this->scopes.size()-1; s_index >= 0; s_index--) {
		bool still_used = false;
		for (int is_index = 0; is_index < (int)this->scopes.size()-1; is_index++) {
			if (s_index != is_index) {
				for (map<int, AbstractNode*>::iterator it = this->scopes[is_index]->nodes.begin();
						it != this->scopes[is_index]->nodes.end(); it++) {
					if (it->second->type == NODE_TYPE_SCOPE) {
						ScopeNode* scope_node = (ScopeNode*)it->second;
						if (scope_node->scope == this->scopes[s_index]) {
							still_used = true;
							break;
						}
					}
				}
			}

			if (still_used) {
				break;
			}
		}

		if (!still_used) {
			delete this->scopes[s_index];
			this->scopes.erase(this->scopes.begin() + s_index);
		}
	}

	for (int s_index = (int)this->scopes.size()-1; s_index >= 0; s_index--) {
		if (this->scopes[s_index]->nodes.size() <= 3) {
			ActionNode* start_node = (ActionNode*)this->scopes[s_index]->nodes[0];
			if (start_node->next_node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)start_node->next_node;
				if (action_node->action.move == ACTION_NOOP) {
					clean_scope_node(this->scopes[s_index]);
				} else {
					clean_scope_node(this->scopes[s_index],
									 action_node->action);
				}
			} else if (start_node->next_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)start_node->next_node;
				clean_scope_node(this->scopes[s_index],
								 scope_node->scope);
			} else {
				clean_scope_node(this->scopes[s_index]);
			}

			delete this->scopes[s_index];
			this->scopes.erase(this->scopes.begin() + s_index);
		}
	}
}

void Solution::random_trim() {
	int starting_num_nodes = 0;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		starting_num_nodes += (this->scopes[s_index]->nodes.size() - 2);
	}

	while (true) {
		uniform_int_distribution<int> scope_distribution(0, this->scopes.size()-1);
		Scope* scope = this->scopes[scope_distribution(generator)];
		uniform_int_distribution<int> node_distribution(0, scope->nodes.size()-1);
		AbstractNode* node = next(scope->nodes.begin(), node_distribution(generator))->second;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->id != 0 && action_node->next_node != NULL) {
					clean_scope_node_helper(scope,
											action_node,
											action_node->next_node);

					scope->clean_node(action_node->id);

					scope->nodes.erase(action_node->id);
					delete action_node;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;

				clean_scope_node_helper(scope,
										scope_node,
										scope_node->next_node);

				scope->clean_node(scope_node->id);

				scope->nodes.erase(scope_node->id);
				delete scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;

				uniform_int_distribution<int> branch_distribution(0, 1);
				if (branch_distribution(generator) == 0) {
					clean_scope_node_helper(scope,
											branch_node,
											branch_node->branch_next_node);
				} else {
					clean_scope_node_helper(scope,
											branch_node,
											branch_node->original_next_node);
				}

				scope->nodes.erase(branch_node->id);
				delete branch_node;
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)node;

				clean_scope_node_helper(scope,
										return_node,
										return_node->next_node);

				scope->nodes.erase(return_node->id);
				delete return_node;
			}
			break;
		}

		clean();

		int curr_num_nodes = 0;
		for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
			curr_num_nodes += (this->scopes[s_index]->nodes.size() - 2);
		}
		if (curr_num_nodes <= (1.0 - TRIM_PERCENTAGE) * starting_num_nodes) {
			break;
		}
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->average_score << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->max_num_actions << endl;

	output_file.close();

	string oldname = path + "saves/" + name + "_temp.txt";
	string newname = path + "saves/" + name + ".txt";
	rename(oldname.c_str(), newname.c_str());
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
