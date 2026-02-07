#include "sum_tree_node.h"

#include "sum_tree.h"

using namespace std;

double SumTreeNode::activate(std::vector<double>& obs,
							 double previous_val) {
	double sum_vals = this->constant;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		sum_vals += this->input_weights[i_index] * obs[this->input_indexes[i_index]];
	}
	sum_vals += this->previous_weight * previous_val;

	return sum_vals;
}

void SumTreeNode::save(ofstream& output_file) {
	output_file << this->constant << endl;
	output_file << this->input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		output_file << this->input_indexes[i_index] << endl;
		output_file << this->input_weights[i_index] << endl;
	}
	output_file << this->previous_weight << endl;

	output_file << this->has_split << endl;
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;
}

void SumTreeNode::load(ifstream& input_file) {
	string constant_line;
	getline(input_file, constant_line);
	this->constant = stod(constant_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->input_indexes.push_back(stoi(index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->input_weights.push_back(stod(weight_line));
	}

	string previous_weight_line;
	getline(input_file, previous_weight_line);
	this->previous_weight = stod(previous_weight_line);

	string has_split_line;
	getline(input_file, has_split_line);
	this->has_split = stoi(has_split_line);

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string rel_obs_index_line;
	getline(input_file, rel_obs_index_line);
	this->rel_obs_index = stoi(rel_obs_index_line);

	string split_type_line;
	getline(input_file, split_type_line);
	this->split_type = stoi(split_type_line);

	string split_target_line;
	getline(input_file, split_target_line);
	this->split_target = stod(split_target_line);

	string split_range_line;
	getline(input_file, split_range_line);
	this->split_range = stod(split_range_line);

	string original_node_id_line;
	getline(input_file, original_node_id_line);
	this->original_node_id = stoi(original_node_id_line);

	string branch_node_id_line;
	getline(input_file, branch_node_id_line);
	this->branch_node_id = stoi(branch_node_id_line);
}

void SumTreeNode::link(SumTree* sum_tree) {
	if (this->original_node_id == -1) {
		this->original_node = NULL;
	} else {
		this->original_node = sum_tree->nodes[this->original_node_id];
	}
	if (this->branch_node_id == -1) {
		this->branch_node = NULL;
	} else {
		this->branch_node = sum_tree->nodes[this->branch_node_id];
	}
}

void SumTreeNode::copy_from(SumTreeNode* original) {
	this->constant = original->constant;
	this->input_indexes = original->input_indexes;
	this->input_weights = original->input_weights;
	this->previous_weight = original->previous_weight;

	this->has_split = original->has_split;
	this->obs_index = original->obs_index;
	this->rel_obs_index = original->rel_obs_index;
	this->split_type = original->split_type;
	this->split_target = original->split_target;
	this->split_range = original->split_range;

	this->original_node_id = original->original_node_id;
	this->branch_node_id = original->branch_node_id;

	this->obs_histories = original->obs_histories;
	this->target_val_histories = original->target_val_histories;
}
