#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "return_node.h"
#include "sample.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double SUBPROBLEM_MIN_INSTANCES_PER_RUN = 0.1;

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

	if (original->subproblem == NULL) {
		this->subproblem = NULL;
		this->subproblem_starting_node = NULL;
		this->subproblem_is_branch = false;
		this->subproblem_exit_node = NULL;
	} else {
		this->subproblem = this->scopes[original->subproblem->id];

		Scope* scope = this->scopes[original->subproblem_starting_node->parent->id];
		this->subproblem_starting_node = scope->nodes[original->subproblem_starting_node->id];
		this->subproblem_is_branch = original->subproblem_is_branch;
		if (original->subproblem_exit_node == NULL) {
			this->subproblem_exit_node = NULL;
		} else {
			this->subproblem_exit_node = scope->nodes[original->subproblem_exit_node->id];
		}
	}
	this->merge_timestamp = -1;
	this->existing_average_score = original->existing_average_score;
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

	this->subproblem = NULL;
	this->subproblem_starting_node = NULL;
	this->subproblem_is_branch = false;
	this->subproblem_exit_node = NULL;
	this->merge_timestamp = -1;
	this->existing_average_score = -1.0;
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

	string subproblem_id_line;
	getline(input_file, subproblem_id_line);
	int subproblem_id = stoi(subproblem_id_line);
	if (subproblem_id == -1) {
		this->subproblem = NULL;
		this->subproblem_starting_node = NULL;
		this->subproblem_is_branch = false;
		this->subproblem_exit_node = NULL;
	} else {
		this->subproblem = this->scopes[subproblem_id];

		string scope_id_line;
		getline(input_file, scope_id_line);
		Scope* scope = this->scopes[stoi(scope_id_line)];

		string starting_node_id_line;
		getline(input_file, starting_node_id_line);
		this->subproblem_starting_node = scope->nodes[stoi(starting_node_id_line)];

		string is_branch_line;
		getline(input_file, is_branch_line);
		this->subproblem_is_branch = stoi(is_branch_line);

		string exit_node_id_line;
		getline(input_file, exit_node_id_line);
		int exit_node_id = stoi(exit_node_id_line);
		if (exit_node_id == -1) {
			this->subproblem_exit_node = NULL;
		} else {
			this->subproblem_exit_node = scope->nodes[exit_node_id];
		}
	}

	string merge_timestamp_line;
	getline(input_file, merge_timestamp_line);
	this->merge_timestamp = stoi(merge_timestamp_line);

	string existing_average_score_line;
	getline(input_file, existing_average_score_line);
	this->existing_average_score = stod(existing_average_score_line);

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
	if (this->subproblem == NULL) {
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

					delete this->scopes[s_index];
					this->scopes.erase(this->scopes.begin() + s_index);
				} else if (start_node->next_node->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)start_node->next_node;
					clean_scope_node(this,
									 this->scopes[s_index],
									 scope_node->scope);

					delete this->scopes[s_index];
					this->scopes.erase(this->scopes.begin() + s_index);
				} else {
					clean_scope_node(this,
									 this->scopes[s_index]);

					delete this->scopes[s_index];
					this->scopes.erase(this->scopes.begin() + s_index);
				}
			}
		}

		for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
			this->scopes[s_index]->id = s_index;
		}
	}
}

void Solution::create_subproblem(string path) {
	uniform_int_distribution<int> subproblem_distribution(0, 9);
	if (this->subproblem == NULL
			&& subproblem_distribution(generator) == 0) {
		while (true) {
			uniform_int_distribution<int> starting_scope_distribution(0, this->scopes.size()-1);
			Scope* potential_parent_scope = this->scopes[starting_scope_distribution(generator)];
			uniform_int_distribution<int> starting_node_distribution(0, potential_parent_scope->nodes.size()-1);
			AbstractNode* potential_starting_node = next(potential_parent_scope->nodes.begin(), starting_node_distribution(generator))->second;
			uniform_int_distribution<int> branch_distribution(0, 1);
			bool potential_is_branch = branch_distribution(generator) == 0;

			AbstractNode* next_node;
			switch (potential_starting_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)potential_starting_node;

					next_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)potential_starting_node;

					next_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)potential_starting_node;

					if (potential_is_branch) {
						next_node = branch_node->branch_next_node;
					} else {
						next_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_RETURN:
				{
					ReturnNode* return_node = (ReturnNode*)potential_starting_node;

					if (potential_is_branch) {
						next_node = return_node->passed_next_node;
					} else {
						next_node = return_node->skipped_next_node;
					}
				}
				break;
			}

			bool can_be_hit = true;
			if (next_node != NULL) {
				if (next_node->average_instances_per_run < SUBPROBLEM_MIN_INSTANCES_PER_RUN) {
					can_be_hit = false;
				}
			}
			if (potential_starting_node->average_instances_per_run < SUBPROBLEM_MIN_INSTANCES_PER_RUN) {
				can_be_hit = false;
			}

			if (can_be_hit) {
				this->subproblem_starting_node = potential_starting_node;
				this->subproblem_is_branch = potential_is_branch;

				break;
			}
		}

		Scope* parent_scope = this->subproblem_starting_node->parent;

		vector<AbstractNode*> possible_exits;

		if (this->subproblem_starting_node->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->subproblem_starting_node)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* starting_node;
		switch (this->subproblem_starting_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->subproblem_starting_node;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->subproblem_starting_node;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->subproblem_starting_node;
				if (this->subproblem_is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)this->subproblem_starting_node;
				if (this->subproblem_is_branch) {
					starting_node = return_node->passed_next_node;
				} else {
					starting_node = return_node->skipped_next_node;
				}
			}
			break;
		}

		parent_scope->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
		int random_index = exit_distribution(generator);
		this->subproblem_exit_node = possible_exits[random_index];

		Scope* new_scope = new Scope();
		new_scope->id = this->scopes.size();
		new_scope->node_counter = 0;
		this->scopes.push_back(new_scope);

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = new_scope;
		starting_noop_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->average_instances_per_run = 1.0;
		new_scope->nodes[starting_noop_node->id] = starting_noop_node;

		ActionNode* previous_node = starting_noop_node;

		uniform_int_distribution<int> sample_distribution(0, 199);
		Sample sample(path,
					  sample_distribution(generator));

		uniform_int_distribution<int> distribution(0, sample.actions.size());
		int index_0 = distribution(generator);
		int index_1 = distribution(generator);
		int start_index;
		int end_index;
		if (index_0 > index_1) {
			start_index = index_1;
			end_index = index_0;
		} else {
			start_index = index_0;
			end_index = index_1;
		}
		for (int a_index = start_index; a_index < end_index; a_index++) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = new_scope;
			new_action_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_action_node->action = sample.actions[a_index];
			new_action_node->average_instances_per_run = 1.0;
			new_scope->nodes[new_action_node->id] = new_action_node;

			previous_node->next_node_id = new_action_node->id;
			previous_node->next_node = new_action_node;

			previous_node = new_action_node;
		}

		ActionNode* end_node = new ActionNode();
		end_node->parent = new_scope;
		end_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		end_node->action = Action(ACTION_NOOP);
		end_node->average_instances_per_run = 1.0;
		new_scope->nodes[end_node->id] = end_node;

		previous_node->next_node_id = end_node->id;
		previous_node->next_node = end_node;

		end_node->next_node_id = -1;
		end_node->next_node = NULL;

		this->subproblem = new_scope;

		uniform_int_distribution<int> merge_distribution(5, 20);
		this->merge_timestamp = this->timestamp + merge_distribution(generator);
		this->existing_average_score = this->average_score;
	}
}

void Solution::merge_subproblem() {
	if (this->subproblem != NULL
			&& this->timestamp >= this->merge_timestamp) {
		if (this->average_score > this->existing_average_score) {
			Scope* parent_scope = this->subproblem_starting_node->parent;

			AbstractNode* end_node;
			for (map<int, AbstractNode*>::iterator it = this->subproblem->nodes.begin();
					it != this->subproblem->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)it->second;
					if (action_node->next_node == NULL) {
						end_node = action_node;
						break;
					}
				}
			}

			AbstractNode* exit_node;
			if (this->subproblem_exit_node == NULL) {
				ActionNode* new_ending_node = new ActionNode();
				new_ending_node->parent = parent_scope;
				new_ending_node->id = parent_scope->node_counter;
				parent_scope->node_counter++;
				parent_scope->nodes[new_ending_node->id] = new_ending_node;

				new_ending_node->action = Action(ACTION_NOOP);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;

				exit_node = new_ending_node;
			} else {
				exit_node = this->subproblem_exit_node;
			}

			for (map<int, AbstractNode*>::iterator it = this->subproblem->nodes.begin();
					it != this->subproblem->nodes.end(); it++) {
				if (it->first != 0
						&& it->second != end_node) {
					it->second->parent = parent_scope;
					it->second->id = parent_scope->node_counter;
					parent_scope->node_counter++;
					parent_scope->nodes[it->second->id] = it->second;
				}
			}

			for (map<int, AbstractNode*>::iterator it = this->subproblem->nodes.begin();
					it != this->subproblem->nodes.end(); it++) {
				if (it->first != 0
						&& it->second != end_node) {
					switch (it->second->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->second;

							if (action_node->next_node == end_node) {
								action_node->next_node_id = exit_node->id;
								action_node->next_node = exit_node;
							} else {
								action_node->next_node_id = action_node->next_node->id;
							}
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)it->second;

							if (scope_node->next_node == end_node) {
								scope_node->next_node_id = exit_node->id;
								scope_node->next_node = exit_node;
							} else {
								scope_node->next_node_id = scope_node->next_node->id;
							}
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)it->second;

							if (branch_node->original_next_node == end_node) {
								branch_node->original_next_node_id = exit_node->id;
								branch_node->original_next_node = exit_node;
							} else {
								branch_node->original_next_node_id = branch_node->original_next_node->id;
							}
							if (branch_node->branch_next_node == end_node) {
								branch_node->branch_next_node_id = exit_node->id;
								branch_node->branch_next_node = exit_node;
							} else {
								branch_node->branch_next_node_id = branch_node->branch_next_node->id;
							}
						}
						break;
					case NODE_TYPE_RETURN:
						{
							ReturnNode* return_node = (ReturnNode*)it->second;

							if (return_node->previous_location_id == 0) {
								return_node->previous_location_id = this->subproblem_starting_node->id;
								return_node->previous_location = this->subproblem_starting_node;
							} else {
								return_node->previous_location_id = return_node->previous_location->id;
							}

							if (return_node->passed_next_node == end_node) {
								return_node->passed_next_node_id = exit_node->id;
								return_node->passed_next_node = exit_node;
							} else {
								return_node->passed_next_node_id = return_node->passed_next_node->id;
							}
							if (return_node->skipped_next_node == end_node) {
								return_node->skipped_next_node_id = exit_node->id;
								return_node->skipped_next_node = exit_node;
							} else {
								return_node->skipped_next_node_id = return_node->skipped_next_node->id;
							}
						}
						break;
					}
				}
			}

			AbstractNode* start_node = ((ActionNode*)subproblem->nodes[0])->next_node;
			switch (this->subproblem_starting_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->subproblem_starting_node;

					action_node->next_node_id = start_node->id;
					action_node->next_node = start_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->subproblem_starting_node;

					scope_node->next_node_id = start_node->id;
					scope_node->next_node = start_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->subproblem_starting_node;

					if (this->subproblem_is_branch) {
						branch_node->branch_next_node_id = start_node->id;
						branch_node->branch_next_node = start_node;
					} else {
						branch_node->original_next_node_id = start_node->id;
						branch_node->original_next_node = start_node;
					}
				}
				break;
			case NODE_TYPE_RETURN:
				{
					ReturnNode* return_node = (ReturnNode*)this->subproblem_starting_node;

					if (this->subproblem_is_branch) {
						return_node->passed_next_node_id = start_node->id;
						return_node->passed_next_node = start_node;
					} else {
						return_node->skipped_next_node_id = start_node->id;
						return_node->skipped_next_node = start_node;
					}
				}
				break;
			}

			delete this->subproblem->nodes[0];
			delete end_node;
			this->subproblem->nodes.clear();

			delete this->subproblem;
			this->scopes.pop_back();

			clean_scope(parent_scope);
			clean();
		} else {
			delete this->subproblem;
			this->scopes.pop_back();
		}

		this->subproblem = NULL;
		this->subproblem_starting_node = NULL;
		this->subproblem_is_branch = false;
		this->subproblem_exit_node = NULL;
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

	if (this->subproblem == NULL) {
		output_file << -1 << endl;
	} else {
		output_file << this->subproblem->id << endl;

		output_file << this->subproblem_starting_node->parent->id << endl;
		output_file << this->subproblem_starting_node->id << endl;
		output_file << this->subproblem_is_branch << endl;
		if (this->subproblem_exit_node == NULL) {
			output_file << -1 << endl;
		} else {
			output_file << this->subproblem_exit_node->id << endl;
		}
	}
	output_file << this->merge_timestamp << endl;
	output_file << this->existing_average_score << endl;

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
