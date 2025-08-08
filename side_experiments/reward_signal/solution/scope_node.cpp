#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

// temp
#include "problem.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->miss_average_guess = 0.0;

	this->is_init = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}

	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		delete this->explore_scope_histories[h_index];
	}
}

void ScopeNode::clean_inputs(Scope* scope,
							 int node_id) {
	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		this->signals[s_index].clean_inputs(scope,
											node_id);
	}
}

void ScopeNode::clean_inputs(Scope* scope) {
	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		this->signals[s_index].clean_inputs(scope);
	}
}

void ScopeNode::replace_obs_node(Scope* scope,
								 int original_node_id,
								 int new_node_id) {
	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		this->signals[s_index].replace_obs_node(scope,
												original_node_id,
												new_node_id);
	}
}

void ScopeNode::clean() {
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		delete this->explore_scope_histories[h_index];
	}
	this->explore_scope_histories.clear();
	this->explore_target_val_histories.clear();
	// temp
	for (int p_index = 0; p_index < (int)this->explore_problems_starts.size(); p_index++) {
		delete this->explore_problems_starts[p_index];
	}
	this->explore_problems_starts.clear();
	for (int p_index = 0; p_index < (int)this->explore_problems_ends.size(); p_index++) {
		delete this->explore_problems_ends[p_index];
	}
	this->explore_problems_ends.clear();

	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	this->last_updated_run_index = -1;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ScopeNode::measure_update(int total_count) {
	this->average_hits_per_run = (double)this->sum_hits / (double)total_count;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;

	output_file << this->signals.size() << endl;
	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		this->signals[s_index].save(output_file);
	}
	output_file << this->miss_average_guess << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ScopeNode::load(ifstream& input_file,
					 Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_signals_line;
	getline(input_file, num_signals_line);
	int num_signals = stoi(num_signals_line);
	for (int s_index = 0; s_index < num_signals; s_index++) {
		this->signals.push_back(Signal(input_file,
									   parent_solution));
	}

	string miss_average_guess_line;
	getline(input_file, miss_average_guess_line);
	this->miss_average_guess = stod(miss_average_guess_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	this->is_init = true;
}

void ScopeNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->signal_initialized = false;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->scope_history = new ScopeHistory(original->scope_history);

	this->signal_initialized = false;
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;
}
