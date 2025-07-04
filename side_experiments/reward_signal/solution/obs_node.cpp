#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->is_init = false;

	this->experiment = NULL;
	this->confusion = NULL;

	this->last_updated_run_index = 0;
}

ObsNode::~ObsNode() {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}

	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}

	if (this->confusion != NULL) {
		delete this->confusion;
	}
}

void ObsNode::clean_inputs(Scope* scope,
						   int node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope,
											 node_id);
	}
}

void ObsNode::clean_inputs(Scope* scope) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope);
	}
}

void ObsNode::replace_factor(Scope* scope,
							 int original_node_id,
							 int original_factor_index,
							 int new_node_id,
							 int new_factor_index) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_factor(scope,
											   original_node_id,
											   original_factor_index,
											   new_node_id,
											   new_factor_index);
	}
}

void ObsNode::replace_obs_node(Scope* scope,
							   int original_node_id,
							   int new_node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_obs_node(scope,
												 original_node_id,
												 new_node_id);
	}
}

void ObsNode::replace_scope(Scope* original_scope,
							Scope* new_scope,
							int new_scope_node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_scope(original_scope,
											  new_scope,
											  new_scope_node_id);
	}
}

void ObsNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	if (this->confusion != NULL) {
		delete this->confusion;
		this->confusion = NULL;
	}

	this->sum_score = 0.0;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ObsNode::measure_update(SolutionWrapper* wrapper) {
	this->average_hits_per_run = (double)this->sum_hits / (double)MEASURE_ITERS;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
	this->average_score = this->sum_score / (double)this->sum_hits;

	this->obs_averages = vector<double>(wrapper->num_obs);
	this->obs_standard_deviations = vector<double>(wrapper->num_obs);
	for (int o_index = 0; o_index < wrapper->num_obs; o_index++) {
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->obs_val_histories.size(); h_index++) {
			sum_vals += this->obs_val_histories[h_index][o_index];
		}
		this->obs_averages[o_index] = sum_vals / (double)this->obs_val_histories.size();

		double sum_variances = 0.0;
		for (int h_index = 0; h_index < (int)this->obs_val_histories.size(); h_index++) {
			sum_variances += (this->obs_val_histories[h_index][o_index] - this->obs_averages[o_index])
				* (this->obs_val_histories[h_index][o_index] - this->obs_averages[o_index]);
		}
		this->obs_standard_deviations[o_index] = sqrt(sum_variances / (double)this->obs_val_histories.size());
	}
	this->obs_val_histories.clear();
}

void ObsNode::new_scope_clean() {
	this->new_scope_sum_score = 0.0;
	this->new_scope_sum_count = 0;
}

void ObsNode::new_scope_measure_update(int total_count) {
	this->new_scope_average_hits_per_run = (double)this->new_scope_sum_count / (double)total_count;
	this->new_scope_average_score = this->new_scope_sum_score / (double)this->new_scope_sum_count;
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average_hits_per_run << endl;
	output_file << this->average_score << endl;

	output_file << this->obs_averages.size() << endl;
	for (int o_index = 0; o_index < (int)this->obs_averages.size(); o_index++) {
		output_file << this->obs_averages[o_index] << endl;
		output_file << this->obs_standard_deviations[o_index] << endl;
	}
}

void ObsNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		Factor* factor = new Factor();
		factor->load(input_file,
					 parent_solution);
		this->factors.push_back(factor);
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	string average_hits_per_run_line;
	getline(input_file, average_hits_per_run_line);
	this->average_hits_per_run = stod(average_hits_per_run_line);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	this->is_init = true;

	cout << "this->id: " << this->id << endl;
	string num_obs_line;
	getline(input_file, num_obs_line);
	int num_obs = stoi(num_obs_line);
	for (int o_index = 0; o_index < num_obs; o_index++) {
		cout << o_index << endl;

		string average_line;
		getline(input_file, average_line);
		this->obs_averages.push_back(stod(average_line));
		cout << "average: " << this->obs_averages[o_index] << endl;

		string standard_deviation_line;
		getline(input_file, standard_deviation_line);
		this->obs_standard_deviations.push_back(stod(standard_deviation_line));
		cout << "standard_deviation: " << this->obs_standard_deviations[o_index] << endl;
	}
	cout << endl;
}

void ObsNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}

ObsNodeHistory::ObsNodeHistory(ObsNodeHistory* original) {
	this->node = original->node;

	this->obs_history = original->obs_history;

	this->factor_initialized = original->factor_initialized;
	this->factor_values = original->factor_values;
}
