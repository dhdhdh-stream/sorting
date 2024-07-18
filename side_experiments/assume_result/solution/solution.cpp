#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->generation = original->generation;

	this->last_updated_scope_id = -1;
	this->last_new_scope_id = -1;

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
	this->last_updated_scope_id = 0;
	this->last_new_scope_id = -1;

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

void Solution::load(ifstream& input_file) {
	string generation_line;
	getline(input_file, generation_line);
	this->generation = stoi(generation_line);

	string last_updated_scope_id_line;
	getline(input_file, last_updated_scope_id_line);
	this->last_updated_scope_id = stoi(last_updated_scope_id_line);

	string last_new_scope_id_line;
	getline(input_file, last_new_scope_id_line);
	this->last_new_scope_id = stoi(last_new_scope_id_line);

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
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::merge_and_delete(Solution* original_solution) {
	for (int s_index = 1; s_index < (int)original_solution->scopes.size(); s_index++) {
		original_solution->scopes[s_index]->id = (int)this->scopes.size();
		this->scopes.push_back(original_solution->scopes[s_index]);
	}
	delete original_solution->scopes[0];

	original_solution->scopes.clear();

	if (original_solution->max_num_actions > this->max_num_actions) {
		this->max_num_actions = original_solution->max_num_actions;
	}
	if (original_solution->num_actions_limit > this->num_actions_limit) {
		this->num_actions_limit = original_solution->num_actions_limit;
	}

	delete original_solution;
}

void Solution::save(ofstream& output_file) {
	output_file << this->generation << endl;

	output_file << this->last_updated_scope_id << endl;
	output_file << this->last_new_scope_id << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->max_num_actions << endl;
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
