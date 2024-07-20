#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

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

	/**
	 * - good to start with extra scopes/structure
	 *   - as scopes[0] will not be reused
	 */

	Scope* base = new Scope();
	base->id = this->scopes.size();
	base->node_counter = 0;
	this->scopes.push_back(base);

	Scope* first_layer = new Scope();
	first_layer->id = this->scopes.size();
	first_layer->node_counter = 0;
	this->scopes.push_back(first_layer);

	Scope* second_layer = new Scope();
	second_layer->id = this->scopes.size();
	second_layer->node_counter = 0;
	this->scopes.push_back(second_layer);

	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = second_layer;
		starting_noop_node->id = second_layer->node_counter;
		second_layer->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP, 0);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		starting_noop_node->average_instances_per_run = 9.0;
		second_layer->nodes[starting_noop_node->id] = starting_noop_node;
	}

	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = first_layer;
		starting_noop_node->id = first_layer->node_counter;
		first_layer->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP, 0);
		starting_noop_node->average_instances_per_run = 3.0;
		first_layer->nodes[starting_noop_node->id] = starting_noop_node;

		ScopeNode* first_scope_node = new ScopeNode();
		first_scope_node->parent = first_layer;
		first_scope_node->id = first_layer->node_counter;
		first_layer->node_counter++;
		first_scope_node->scope = second_layer;
		first_scope_node->average_instances_per_run = 3.0;
		first_layer->nodes[first_scope_node->id] = first_scope_node;

		ScopeNode* second_scope_node = new ScopeNode();
		second_scope_node->parent = first_layer;
		second_scope_node->id = first_layer->node_counter;
		first_layer->node_counter++;
		second_scope_node->scope = second_layer;
		second_scope_node->average_instances_per_run = 3.0;
		first_layer->nodes[second_scope_node->id] = second_scope_node;

		ScopeNode* third_scope_node = new ScopeNode();
		third_scope_node->parent = first_layer;
		third_scope_node->id = first_layer->node_counter;
		first_layer->node_counter++;
		third_scope_node->scope = second_layer;
		third_scope_node->average_instances_per_run = 3.0;
		first_layer->nodes[third_scope_node->id] = third_scope_node;

		ActionNode* ending_node = new ActionNode();
		ending_node->parent = first_layer;
		ending_node->id = first_layer->node_counter;
		first_layer->node_counter++;
		ending_node->action = Action(ACTION_NOOP, 0);
		ending_node->average_instances_per_run = 3.0;
		first_layer->nodes[ending_node->id] = ending_node;

		starting_noop_node->next_node_id = first_scope_node->id;
		starting_noop_node->next_node = first_scope_node;

		first_scope_node->next_node_id = second_scope_node->id;
		first_scope_node->next_node = second_scope_node;

		second_scope_node->next_node_id = third_scope_node->id;
		second_scope_node->next_node = third_scope_node;

		third_scope_node->next_node_id = ending_node->id;
		third_scope_node->next_node = ending_node;

		ending_node->next_node_id = -1;
		ending_node->next_node = NULL;
	}

	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = base;
		starting_noop_node->id = base->node_counter;
		base->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP, 0);
		starting_noop_node->average_instances_per_run = 1.0;
		base->nodes[starting_noop_node->id] = starting_noop_node;

		ScopeNode* first_scope_node = new ScopeNode();
		first_scope_node->parent = base;
		first_scope_node->id = base->node_counter;
		base->node_counter++;
		first_scope_node->scope = first_layer;
		first_scope_node->average_instances_per_run = 1.0;
		base->nodes[first_scope_node->id] = first_scope_node;

		ScopeNode* second_scope_node = new ScopeNode();
		second_scope_node->parent = base;
		second_scope_node->id = base->node_counter;
		base->node_counter++;
		second_scope_node->scope = first_layer;
		second_scope_node->average_instances_per_run = 1.0;
		base->nodes[second_scope_node->id] = second_scope_node;

		ScopeNode* third_scope_node = new ScopeNode();
		third_scope_node->parent = base;
		third_scope_node->id = base->node_counter;
		base->node_counter++;
		third_scope_node->scope = first_layer;
		third_scope_node->average_instances_per_run = 1.0;
		base->nodes[third_scope_node->id] = third_scope_node;

		ActionNode* ending_node = new ActionNode();
		ending_node->parent = base;
		ending_node->id = base->node_counter;
		base->node_counter++;
		ending_node->action = Action(ACTION_NOOP, 0);
		ending_node->average_instances_per_run = 1.0;
		base->nodes[ending_node->id] = ending_node;

		starting_noop_node->next_node_id = first_scope_node->id;
		starting_noop_node->next_node = first_scope_node;

		first_scope_node->next_node_id = second_scope_node->id;
		first_scope_node->next_node = second_scope_node;

		second_scope_node->next_node_id = third_scope_node->id;
		second_scope_node->next_node = third_scope_node;

		third_scope_node->next_node_id = ending_node->id;
		third_scope_node->next_node = ending_node;

		ending_node->next_node_id = -1;
		ending_node->next_node = NULL;
	}

	// this->max_num_actions = 1;
	this->max_num_actions = 9;
	// this->num_actions_limit = 40;
	this->num_actions_limit = 400;
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
