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

	this->experiment = NULL;
}

Eval::Eval(Eval* original,
		   Solution* parent_solution) {
	this->parent_scope = parent_solution->scopes[original->parent_scope->id];

	this->subscope = new Scope();
	this->subscope->copy_from(original->subscope,
							  parent_solution);
	this->subscope->link(parent_solution);

	this->score_average_score = original->score_average_score;

	this->score_input_obs_indexes = original->score_input_obs_indexes;
	for (int i_index = 0; i_index < (int)original->score_input_node_contexts.size(); i_index++) {
		this->score_input_node_contexts.push_back(
			this->subscope->nodes[original->score_input_node_contexts[i_index]->id]);
	}

	this->score_linear_input_indexes = original->score_linear_input_indexes;
	this->score_linear_weights = original->score_linear_weights;

	this->score_network_input_indexes = original->score_network_input_indexes;
	if (original->score_network == NULL) {
		this->score_network = NULL;
	} else {
		this->score_network = new Network(original->score_network);
	}

	this->vs_average_score = original->vs_average_score;

	this->vs_input_is_start = original->vs_input_is_start;
	this->vs_input_obs_indexes = original->vs_input_obs_indexes;
	for (int i_index = 0; i_index < (int)original->vs_input_node_contexts.size(); i_index++) {
		this->vs_input_node_contexts.push_back(
			this->subscope->nodes[original->vs_input_node_contexts[i_index]->id]);
	}

	this->vs_linear_input_indexes = original->vs_linear_input_indexes;
	this->vs_linear_weights = original->vs_linear_weights;

	this->vs_network_input_indexes = original->vs_network_input_indexes;
	if (original->vs_network == NULL) {
		this->vs_network = NULL;
	} else {
		this->vs_network = new Network(original->vs_network);
	}

	this->experiment = NULL;
}

Eval::~Eval() {
	delete this->subscope;

	if (this->score_network != NULL) {
		delete this->score_network;
	}
	if (this->vs_network != NULL) {
		delete this->vs_network;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
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

	this->score_average_score = 0.0;
	this->vs_average_score = 0.0;

	this->score_network = NULL;
	this->vs_network = NULL;
}

void Eval::load(ifstream& input_file) {
	this->subscope = new Scope();
	this->subscope->id = -1;
	this->subscope->load(input_file);
	this->subscope->link(solution);

	string score_average_score_line;
	getline(input_file, score_average_score_line);
	this->score_average_score = stod(score_average_score_line);

	string score_num_inputs_line;
	getline(input_file, score_num_inputs_line);
	int score_num_inputs = stoi(score_num_inputs_line);
	for (int i_index = 0; i_index < score_num_inputs; i_index++) {
		string node_context_id;
		getline(input_file, node_context_id);
		this->score_input_node_contexts.push_back(
			this->subscope->nodes[stoi(node_context_id)]);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->score_input_obs_indexes.push_back(stoi(obs_index_line));
	}

	string score_num_linear_inputs_line;
	getline(input_file, score_num_linear_inputs_line);
	int score_num_linear_inputs = stoi(score_num_linear_inputs_line);
	for (int i_index = 0; i_index < score_num_linear_inputs; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->score_linear_input_indexes.push_back(stoi(input_index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->score_linear_weights.push_back(stod(weight_line));
	}

	string score_num_network_inputs_line;
	getline(input_file, score_num_network_inputs_line);
	int score_num_network_inputs = stoi(score_num_network_inputs_line);
	for (int i_index = 0; i_index < score_num_network_inputs; i_index++) {
		string layer_size_line;
		getline(input_file, layer_size_line);
		int layer_size = stoi(layer_size_line);
		vector<int> v_input_indexes;
		for (int v_index = 0; v_index < layer_size; v_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			v_input_indexes.push_back(stoi(input_index_line));
		}
		this->score_network_input_indexes.push_back(v_input_indexes);
	}
	if (this->score_network_input_indexes.size() == 0) {
		this->score_network = NULL;
	} else {
		this->score_network = new Network(input_file);
	}

	string vs_average_score_line;
	getline(input_file, vs_average_score_line);
	this->vs_average_score = stod(vs_average_score_line);

	string vs_num_inputs_line;
	getline(input_file, vs_num_inputs_line);
	int vs_num_inputs = stoi(vs_num_inputs_line);
	for (int i_index = 0; i_index < vs_num_inputs; i_index++) {
		string is_start_line;
		getline(input_file, is_start_line);
		this->vs_input_is_start.push_back(stoi(is_start_line));

		string node_context_id;
		getline(input_file, node_context_id);
		this->vs_input_node_contexts.push_back(
			this->subscope->nodes[stoi(node_context_id)]);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->vs_input_obs_indexes.push_back(stoi(obs_index_line));
	}

	string vs_num_linear_inputs_line;
	getline(input_file, vs_num_linear_inputs_line);
	int vs_num_linear_inputs = stoi(vs_num_linear_inputs_line);
	for (int i_index = 0; i_index < vs_num_linear_inputs; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->vs_linear_input_indexes.push_back(stoi(input_index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->vs_linear_weights.push_back(stod(weight_line));
	}

	string vs_num_network_inputs_line;
	getline(input_file, vs_num_network_inputs_line);
	int vs_num_network_inputs = stoi(vs_num_network_inputs_line);
	for (int i_index = 0; i_index < vs_num_network_inputs; i_index++) {
		string layer_size_line;
		getline(input_file, layer_size_line);
		int layer_size = stoi(layer_size_line);
		vector<int> v_input_indexes;
		for (int v_index = 0; v_index < layer_size; v_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			v_input_indexes.push_back(stoi(input_index_line));
		}
		this->vs_network_input_indexes.push_back(v_input_indexes);
	}
	if (this->vs_network_input_indexes.size() == 0) {
		this->vs_network = NULL;
	} else {
		this->vs_network = new Network(input_file);
	}
}

void Eval::save(ofstream& output_file) {
	this->subscope->save(output_file);

	output_file << this->score_average_score << endl;

	output_file << this->score_input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
		output_file << this->score_input_node_contexts[i_index]->id << endl;
		output_file << this->score_input_obs_indexes[i_index] << endl;
	}

	output_file << this->score_linear_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_linear_input_indexes.size(); i_index++) {
		output_file << this->score_linear_input_indexes[i_index] << endl;
		output_file << this->score_linear_weights[i_index] << endl;
	}

	output_file << this->score_network_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
		output_file << this->score_network_input_indexes[i_index].size() << endl;
		for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
			output_file << this->score_network_input_indexes[i_index][v_index] << endl;
		}
	}
	if (this->score_network != NULL) {
		this->score_network->save(output_file);
	}

	output_file << this->vs_average_score << endl;

	output_file << this->vs_input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
		output_file << this->vs_input_is_start[i_index] << endl;
		output_file << this->vs_input_node_contexts[i_index]->id << endl;
		output_file << this->vs_input_obs_indexes[i_index] << endl;
	}

	output_file << this->vs_linear_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->vs_linear_input_indexes.size(); i_index++) {
		output_file << this->vs_linear_input_indexes[i_index] << endl;
		output_file << this->vs_linear_weights[i_index] << endl;
	}

	output_file << this->vs_network_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->vs_network_input_indexes.size(); i_index++) {
		output_file << this->vs_network_input_indexes[i_index].size() << endl;
		for (int v_index = 0; v_index < (int)this->vs_network_input_indexes[i_index].size(); v_index++) {
			output_file << this->vs_network_input_indexes[i_index][v_index] << endl;
		}
	}
	if (this->vs_network != NULL) {
		this->vs_network->save(output_file);
	}
}

EvalHistory::EvalHistory(Eval* eval) {
	this->eval = eval;

	this->start_scope_history = NULL;
	this->end_scope_history = NULL;
}

EvalHistory::EvalHistory(EvalHistory* original) {
	this->eval = original->eval;

	if (original->start_scope_history == NULL) {
		this->start_scope_history = NULL;
	} else {
		this->start_scope_history = new ScopeHistory(original->start_scope_history);
	}

	if (original->end_scope_history == NULL) {
		this->end_scope_history = NULL;
	} else {
		this->end_scope_history = new ScopeHistory(original->end_scope_history);
	}
}

EvalHistory::~EvalHistory() {
	if (this->start_scope_history != NULL) {
		delete this->start_scope_history;
	}

	if (this->end_scope_history != NULL) {
		delete this->end_scope_history;
	}
}
