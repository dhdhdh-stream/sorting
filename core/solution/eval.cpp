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

	this->subscope = new Scope();
	this->subscope->copy_from(original->subscope,
							  parent_solution);
	this->subscope->link(parent_solution);

	this->average_score = original->average_score;

	this->input_obs_indexes = original->input_obs_indexes;
	for (int i_index = 0; i_index < (int)original->input_node_contexts.size(); i_index++) {
		this->input_node_contexts.push_back(
			this->subscope->nodes[original->input_node_contexts[i_index]->id]);
	}

	this->linear_input_indexes = original->linear_input_indexes;
	this->linear_weights = original->linear_weights;

	this->network_input_indexes = original->network_input_indexes;
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
	this->subscope = new Scope();
	this->subscope->id = -1;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = this->subscope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	this->subscope->nodes[0] = starting_noop_node;
	this->subscope->node_counter = 1;

	this->average_score = 0.0;

	this->network = NULL;
}

void Eval::load(ifstream& input_file) {
	this->subscope = new Scope();
	this->subscope->id = -1;
	this->subscope->load(input_file);
	this->subscope->link(solution);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

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

	string num_linear_inputs_line;
	getline(input_file, num_linear_inputs_line);
	int num_linear_inputs = stoi(num_linear_inputs_line);
	for (int i_index = 0; i_index < num_linear_inputs; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->linear_input_indexes.push_back(stoi(input_index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->linear_weights.push_back(stod(weight_line));
	}

	string num_network_inputs_line;
	getline(input_file, num_network_inputs_line);
	int num_network_inputs = stoi(num_network_inputs_line);
	for (int i_index = 0; i_index < num_network_inputs; i_index++) {
		string layer_size_line;
		getline(input_file, layer_size_line);
		int layer_size = stoi(layer_size_line);
		vector<int> v_input_indexes;
		for (int v_index = 0; v_index < layer_size; v_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			v_input_indexes.push_back(stoi(input_index_line));
		}
		this->network_input_indexes.push_back(v_input_indexes);
	}
	if (this->network_input_indexes.size() == 0) {
		this->network = NULL;
	} else {
		this->network = new Network(input_file);
	}
}

void Eval::save(ofstream& output_file) {
	this->subscope->save(output_file);

	output_file << this->average_score << endl;

	output_file << this->input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		output_file << this->input_node_contexts[i_index]->id << endl;
		output_file << this->input_obs_indexes[i_index] << endl;
	}

	output_file << this->linear_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->linear_input_indexes.size(); i_index++) {
		output_file << this->linear_input_indexes[i_index] << endl;
		output_file << this->linear_weights[i_index] << endl;
	}

	output_file << this->network_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
		output_file << this->network_input_indexes[i_index].size() << endl;
		for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
			output_file << this->network_input_indexes[i_index][v_index] << endl;
		}
	}
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
}

EvalHistory::~EvalHistory() {
	if (this->scope_history != NULL) {
		delete this->scope_history;
	}
}
