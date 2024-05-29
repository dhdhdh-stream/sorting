#include "eval.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Eval::Eval(Scope* parent_scope) {
	this->parent_scope = parent_scope;
}

Eval::Eval(Eval* original,
		   Solution* parent_solution) {
	this->parent_scope = parent_solution->scopes[original->parent_scope->id];

	this->subscope = new Scope(this);
	this->subscope->copy_from(original->subscope,
							  parent_solution);
	this->subscope->link(parent_solution);

	for (int i_index = 0; i_index < (int)original->input_node_contexts.size(); i_index++) {
		this->input_node_contexts.push_back(
			this->subscope->nodes[original->input_node_contexts[i_index]->id]);
	}
	this->input_obs_indexes = original->input_obs_indexes;
	if (original->network == NULL) {
		this->network = NULL;
	} else {
		this->network = new Network(original->network);
	}
}

Eval::~Eval() {
	delete this->subscope;

	if (this->network != NULL) {
		delete this->network;
	}
}

void Eval::init() {
	this->subscope = new Scope(this);
	this->subscope->id = -1;

	this->subscope->node_counter = 0;

	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = this->subscope;
		starting_noop_node->id = this->subscope->node_counter;
		this->subscope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		this->subscope->nodes[starting_noop_node->id] = starting_noop_node;
	}
	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = this->subscope;
		starting_noop_node->id = this->subscope->node_counter;
		this->subscope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		this->subscope->nodes[starting_noop_node->id] = starting_noop_node;
	}
	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = this->subscope;
		starting_noop_node->id = this->subscope->node_counter;
		this->subscope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		this->subscope->nodes[starting_noop_node->id] = starting_noop_node;
	}
	{
		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = this->subscope;
		starting_noop_node->id = this->subscope->node_counter;
		this->subscope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		this->subscope->nodes[starting_noop_node->id] = starting_noop_node;
	}

	this->network = NULL;
}

void Eval::load(ifstream& input_file) {
	this->subscope = new Scope(this);
	this->subscope->id = -1;
	this->subscope->load(input_file);
	this->subscope->link(solution);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string node_context_id;
		getline(input_file, node_context_id);
		this->input_node_contexts.push_back(
			this->subscope->nodes[stoi(node_context_id)]);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}

	string network_is_null_line;
	getline(input_file, network_is_null_line);
	bool network_is_null = stoi(network_is_null_line);
	if (network_is_null) {
		this->network = NULL;
	} else {
		this->network = new Network(input_file);
	}
}

void Eval::save(ofstream& output_file) {
	this->subscope->save(output_file);

	output_file << this->input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		output_file << this->input_node_contexts[i_index]->id << endl;
		output_file << this->input_obs_indexes[i_index] << endl;
	}

	bool network_is_null = this->network == NULL;
	output_file << network_is_null << endl;
	if (this->network != NULL) {
		this->network->save(output_file);
	}
}

EvalHistory::EvalHistory(Eval* eval) {
	this->eval = eval;

	this->scope_history = new ScopeHistory(this->eval->subscope);
}

EvalHistory::EvalHistory(EvalHistory* original) {
	this->eval = original->eval;

	this->scope_history = new ScopeHistory(original->scope_history);
	this->start_eval_index = original->start_eval_index;
	this->end_orientation_index = original->end_orientation_index;
	this->end_eval_index = original->end_eval_index;
}

EvalHistory::~EvalHistory() {
	delete this->scope_history;
}
