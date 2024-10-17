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

const double TRIM_PERCENTAGE = 0.15;

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
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_noop_node->average_instances_per_run = 1.0;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	this->max_num_actions = 10;
	this->num_actions_limit = 100;
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

	this->num_actions_limit = 10*this->max_num_actions + 10;

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

	for (int s_index = (int)this->scopes.size()-1; s_index >= 1; s_index--) {
		if (this->scopes[s_index]->nodes.size() <= 3) {
			ActionNode* start_node = (ActionNode*)this->scopes[s_index]->nodes[0];
			if (start_node->next_node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)start_node->next_node;
				if (action_node->action.move == ACTION_NOOP) {
					clean_scope_node(this,
									 this->scopes[s_index]);
				} else {
					clean_scope_node(this,
									 this->scopes[s_index],
									 action_node->action);
				}
			} else if (start_node->next_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)start_node->next_node;
				clean_scope_node(this,
								 this->scopes[s_index],
								 scope_node->scope);
			} else {
				clean_scope_node(this,
								 this->scopes[s_index]);
			}

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

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->id = s_index;
	}
}

// TODO: cleans up solution, but doesn't break local optima
void Solution::random_trim() {
	int starting_num_nodes = 0;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		starting_num_nodes += (this->scopes[s_index]->nodes.size() - 2);
	}

	while (true) {
		vector<Scope*> possible_scopes;
		vector<AbstractNode*> possible_nodes;
		for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
			for (map<int, AbstractNode*>::iterator it = this->scopes[s_index]->nodes.begin();
					it != this->scopes[s_index]->nodes.end(); it++) {
				switch (it->second->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;
						if (action_node->id != 0 && action_node->next_node != NULL) {
							possible_scopes.push_back(this->scopes[s_index]);
							possible_nodes.push_back(it->second);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
				case NODE_TYPE_BRANCH:
				case NODE_TYPE_RETURN:
					possible_scopes.push_back(this->scopes[s_index]);
					possible_nodes.push_back(it->second);
					break;
				}
			}
		}

		uniform_int_distribution<int> random_distribution(0, possible_scopes.size()-1);
		int random_index = random_distribution(generator);
		switch (possible_nodes[random_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)possible_nodes[random_index];

				clean_scope_node_helper(possible_scopes[random_index],
										action_node,
										action_node->next_node);

				possible_scopes[random_index]->clean_node(action_node->id);

				possible_scopes[random_index]->nodes.erase(action_node->id);
				delete action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)possible_nodes[random_index];

				clean_scope_node_helper(possible_scopes[random_index],
										scope_node,
										scope_node->next_node);

				possible_scopes[random_index]->clean_node(scope_node->id);

				possible_scopes[random_index]->nodes.erase(scope_node->id);
				delete scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)possible_nodes[random_index];

				uniform_int_distribution<int> branch_distribution(0, 1);
				if (branch_distribution(generator) == 0) {
					clean_scope_node_helper(possible_scopes[random_index],
											branch_node,
											branch_node->branch_next_node);
				} else {
					clean_scope_node_helper(possible_scopes[random_index],
											branch_node,
											branch_node->original_next_node);
				}

				possible_scopes[random_index]->nodes.erase(branch_node->id);
				delete branch_node;

				clean_scope(possible_scopes[random_index]);
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)possible_nodes[random_index];

				uniform_int_distribution<int> branch_distribution(0, 1);
				if (branch_distribution(generator) == 0) {
					clean_scope_node_helper(possible_scopes[random_index],
											return_node,
											return_node->passed_next_node);
				} else {
					clean_scope_node_helper(possible_scopes[random_index],
											return_node,
											return_node->skipped_next_node);
				}

				possible_scopes[random_index]->nodes.erase(return_node->id);
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
