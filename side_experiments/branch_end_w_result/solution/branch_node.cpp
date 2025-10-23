#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */

	this->original_last_updated_run_index = -1;
	this->branch_last_updated_run_index = -1;

	this->experiment = NULL;
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
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

void BranchNode::clean() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}

	this->original_last_updated_run_index = -1;
	this->original_sum_hits = 0;
	this->original_sum_instances = 0;
	this->branch_last_updated_run_index = -1;
	this->branch_sum_hits = 0;
	this->branch_sum_instances = 0;
}

void BranchNode::measure_update(int total_count) {
	this->original_average_hits_per_run = (double)this->original_sum_hits / (double)total_count;
	this->original_average_instances_per_run = (double)this->original_sum_instances / (double)this->original_sum_hits;
	this->branch_average_hits_per_run = (double)this->branch_sum_hits / (double)total_count;
	this->branch_average_instances_per_run = (double)this->branch_sum_instances / (double)this->branch_sum_hits;
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->constant << endl;
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
		output_file << this->input_averages[i_index] << endl;
		output_file << this->input_standard_deviations[i_index] << endl;
		output_file << this->weights[i_index] << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->branch_end_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string constant_line;
	getline(input_file, constant_line);
	this->constant = stod(constant_line);

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

	string branch_end_node_id_line;
	getline(input_file, branch_end_node_id_line);
	this->branch_end_node_id = stoi(branch_end_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
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

	// temp
	if (this->parent->nodes.find(this->branch_end_node_id) == this->parent->nodes.end()) {
		throw invalid_argument("this->parent->nodes.find(this->branch_end_node_id) == this->parent->nodes.end()");
	}
	this->branch_end_node = (ObsNode*)this->parent->nodes[this->branch_end_node_id];
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
	this->index = original->index;
	this->num_actions_snapshot = original->num_actions_snapshot;

	this->is_branch = original->is_branch;
}
