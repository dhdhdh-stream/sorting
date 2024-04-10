#include "solution.h"

#include <iostream>

#include "action_node.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = (unsigned)time(NULL);
	this->curr_average_score = -1.0;

	Scope* starting_scope = new Scope();
	starting_scope->id = 0;
	this->scopes.push_back(starting_scope);

	starting_scope->parent_id = -1;
	starting_scope->layer = 0;
	starting_scope->num_improvements = 0;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = starting_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_scope->nodes[0] = starting_noop_node;
	starting_scope->default_starting_node_id = 0;
	starting_scope->default_starting_node = starting_noop_node;
	starting_scope->node_counter = 1;

	this->curr_scope_id = 0;

	this->throw_counter = 0;

	this->max_depth = 1;
	this->depth_limit = 11;

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

	string curr_average_score_line;
	getline(input_file, curr_average_score_line);
	this->curr_average_score = stod(curr_average_score_line);

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

	string curr_scope_id_line;
	getline(input_file, curr_scope_id_line);
	this->curr_scope_id = stoi(curr_scope_id_line);

	string throw_counter_line;
	getline(input_file, throw_counter_line);
	this->throw_counter = stoi(throw_counter_line);

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}

	string max_num_actions_line;
	getline(input_file, max_num_actions_line);
	this->max_num_actions = stoi(max_num_actions_line);

	this->num_actions_limit = 20*this->max_num_actions + 20;

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_key = NULL;
	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::increment() {
	this->scopes[this->curr_scope_id]->num_improvements++;

	int num_improvements_target = DEFAULT_NUM_IMPROVEMENTS;
	for (int l_index = 0; l_index < this->scopes[this->curr_scope_id]->layer; l_index++) {
		num_improvements_target *= 2;
	}
	if (this->scopes[this->curr_scope_id]->num_improvements >= num_improvements_target) {
		Scope* curr_scope = this->scopes[this->curr_scope_id];
		while (true) {
			if (curr_scope->layer != 0) {
				if (curr_scope->child_ids.size() < MAX_NUM_CHILDREN
						|| curr_scope->num_improvements == 0) {
					break;
				}
			}

			if (curr_scope->parent_id == -1) {
				Scope* new_ancestor = new Scope();
				new_ancestor->id = this->scopes.size();
				this->scopes.push_back(new_ancestor);

				new_ancestor->parent_id = -1;
				new_ancestor->child_ids.push_back(curr_scope->id);
				new_ancestor->layer = curr_scope->layer+1;
				new_ancestor->num_improvements = 0;

				ActionNode* starting_noop_node = new ActionNode();
				starting_noop_node->parent = new_ancestor;
				starting_noop_node->id = 0;
				starting_noop_node->action = Action(ACTION_NOOP);
				starting_noop_node->next_node_id = -1;
				starting_noop_node->next_node = NULL;
				new_ancestor->nodes[0] = starting_noop_node;
				new_ancestor->default_starting_node_id = 0;
				new_ancestor->default_starting_node = starting_noop_node;
				new_ancestor->node_counter = 1;

				curr_scope->parent_id = new_ancestor->id;

				curr_scope = new_ancestor;

				break;
			}

			curr_scope = this->scopes[curr_scope->parent_id];
		}

		if ((int)curr_scope->child_ids.size() == MAX_NUM_CHILDREN) {
			this->curr_scope_id = curr_scope->id;
		} else {
			while (true) {
				if (curr_scope->layer == 0) {
					this->curr_scope_id = curr_scope->id;
					break;
				}

				Scope* new_child = new Scope();
				new_child->id = this->scopes.size();
				this->scopes.push_back(new_child);

				new_child->parent_id = curr_scope->id;
				new_child->layer = curr_scope->layer-1;
				new_child->num_improvements = 0;

				ActionNode* starting_noop_node = new ActionNode();
				starting_noop_node->parent = new_child;
				starting_noop_node->id = 0;
				starting_noop_node->action = Action(ACTION_NOOP);
				starting_noop_node->next_node_id = -1;
				starting_noop_node->next_node = NULL;
				new_child->nodes[0] = starting_noop_node;
				new_child->default_starting_node_id = 0;
				new_child->default_starting_node = starting_noop_node;
				new_child->node_counter = 1;

				curr_scope->child_ids.push_back(new_child->id);

				curr_scope = new_child;
			}
		}
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->curr_average_score << endl;

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}
	output_file << this->curr_scope_id << endl;

	output_file << this->throw_counter << endl;

	output_file << this->max_depth << endl;

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
