#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->is_init = false;

	this->experiment = NULL;
	this->confusion = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */

	this->original_last_updated_run_index = 0;
	this->branch_last_updated_run_index = 0;
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}

	if (this->confusion != NULL) {
		delete this->confusion;
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::clean_inputs(Scope* scope,
							  int node_id) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope
					&& this->inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->input_averages.erase(this->input_averages.begin() + i_index);
			this->input_standard_deviations.erase(this->input_standard_deviations.begin() + i_index);
			this->weights.erase(this->weights.begin() + i_index);
		}
	}
}

void BranchNode::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->input_averages.erase(this->input_averages.begin() + i_index);
			this->input_standard_deviations.erase(this->input_standard_deviations.begin() + i_index);
			this->weights.erase(this->weights.begin() + i_index);
		}
	}
}

void BranchNode::replace_factor(Scope* scope,
								int original_node_id,
								int original_factor_index,
								int new_node_id,
								int new_factor_index) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id
				&& this->inputs[i_index].factor_index == original_factor_index) {
			this->inputs[i_index].node_context.back() = new_node_id;
			this->inputs[i_index].factor_index = new_factor_index;
		}
	}
}

void BranchNode::replace_obs_node(Scope* scope,
								  int original_node_id,
								  int new_node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id) {
			this->inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void BranchNode::replace_scope(Scope* original_scope,
							   Scope* new_scope,
							   int new_scope_node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 1; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == original_scope) {
				this->inputs[i_index].scope_context.insert(
					this->inputs[i_index].scope_context.begin() + l_index, new_scope);
				this->inputs[i_index].node_context.insert(
					this->inputs[i_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}
}

void BranchNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	if (this->confusion != NULL) {
		delete this->confusion;
		this->confusion = NULL;
	}

	this->original_sum_score = 0.0;
	this->original_sum_hits = 0;
	this->original_sum_instances = 0;
	this->branch_sum_score = 0.0;
	this->branch_sum_hits = 0;
	this->branch_sum_instances = 0;
}

void BranchNode::measure_update() {
	this->original_average_hits_per_run = (double)this->original_sum_hits / (double)MEASURE_ITERS;
	this->original_average_instances_per_run = (double)this->original_sum_instances / (double)this->original_sum_hits;
	this->original_average_score = this->original_sum_score / (double)this->original_sum_hits;
	this->branch_average_hits_per_run = (double)this->branch_sum_hits / (double)MEASURE_ITERS;
	this->branch_average_instances_per_run = (double)this->branch_sum_instances / (double)this->branch_sum_hits;
	this->branch_average_score = this->branch_sum_score / (double)this->branch_sum_hits;
}

void BranchNode::new_scope_clean() {
	this->original_new_scope_sum_score = 0.0;
	this->original_new_scope_sum_count = 0;
	this->branch_new_scope_sum_score = 0.0;
	this->branch_new_scope_sum_count = 0;
}

void BranchNode::new_scope_measure_update(int total_count) {
	this->original_new_scope_average_hits_per_run = (double)this->original_new_scope_sum_count / (double)total_count;
	this->original_new_scope_average_score = this->original_new_scope_sum_score / (double)this->original_new_scope_sum_count;
	this->branch_new_scope_average_hits_per_run = (double)this->branch_new_scope_sum_count / (double)total_count;
	this->branch_new_scope_average_score = this->branch_new_scope_sum_score / (double)this->branch_new_scope_sum_count;
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->average_val << endl;
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
		output_file << this->input_averages[i_index] << endl;
		output_file << this->input_standard_deviations[i_index] << endl;
		output_file << this->weights[i_index] << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->original_average_hits_per_run << endl;
	output_file << this->original_average_score << endl;
	output_file << this->branch_average_hits_per_run << endl;
	output_file << this->branch_average_score << endl;
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->inputs.push_back(Input(input_file,
									 parent_solution));

		string input_average_line;
		getline(input_file, input_average_line);
		this->input_averages.push_back(stod(input_average_line));

		string input_standard_deviation_line;
		getline(input_file, input_standard_deviation_line);
		this->input_standard_deviations.push_back(stod(input_standard_deviation_line));

		string weight_line;
		getline(input_file, weight_line);
		this->weights.push_back(stod(weight_line));
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	string original_average_hits_per_run_line;
	getline(input_file, original_average_hits_per_run_line);
	this->original_average_hits_per_run = stod(original_average_hits_per_run_line);

	string original_average_score_line;
	getline(input_file, original_average_score_line);
	this->original_average_score = stod(original_average_score_line);

	string branch_average_hits_per_run_line;
	getline(input_file, branch_average_hits_per_run_line);
	this->branch_average_hits_per_run = stod(branch_average_hits_per_run_line);

	string branch_average_score_line;
	getline(input_file, branch_average_score_line);
	this->branch_average_score = stod(branch_average_score_line);

	this->is_init = true;
}

void BranchNode::link(Solution* parent_solution) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}

BranchNodeHistory::BranchNodeHistory(BranchNodeHistory* original) {
	this->node = original->node;

	this->is_branch = original->is_branch;
}
