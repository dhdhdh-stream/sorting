#include "decision_tree_node.h"

#include "decision_tree.h"
#include "network.h"

using namespace std;

DecisionTreeNode::DecisionTreeNode() {
	this->network = NULL;
}

DecisionTreeNode::~DecisionTreeNode() {
	if (this->network != NULL) {
		delete this->network;
	}
}

double DecisionTreeNode::activate(std::vector<double>& obs,
								  double previous_val) {
	if (this->is_previous) {
		return previous_val;
	} else {
		vector<double> inputs(1 + this->input_indexes.size());
		inputs[0] = previous_val;
		for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
			inputs[1 + i_index] = obs[this->input_indexes[i_index]];
		}

		this->network->activate(inputs);

		return this->network->output->acti_vals[0];
	}
}

void DecisionTreeNode::save(ofstream& output_file) {
	output_file << this->is_previous << endl;
	output_file << this->input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		output_file << this->input_indexes[i_index] << endl;
	}
	if (!this->is_previous) {
		this->network->save(output_file);
	}

	output_file << this->has_split << endl;
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;
}

void DecisionTreeNode::load(ifstream& input_file) {
	string is_previous_line;
	getline(input_file, is_previous_line);
	this->is_previous = stoi(is_previous_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->input_indexes.push_back(stoi(index_line));
	}

	if (this->is_previous) {
		this->network = NULL;
	} else {
		this->network = new Network(input_file);
	}

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

void DecisionTreeNode::link(DecisionTree* decision_tree) {
	if (this->original_node_id == -1) {
		this->original_node = NULL;
	} else {
		this->original_node = decision_tree->nodes[this->original_node_id];
	}
	if (this->branch_node_id == -1) {
		this->branch_node = NULL;
	} else {
		this->branch_node = decision_tree->nodes[this->branch_node_id];
	}
}

void DecisionTreeNode::copy_from(DecisionTreeNode* original) {
	this->is_previous = original->is_previous;
	this->input_indexes = original->input_indexes;
	if (original->network == NULL) {
		this->network = NULL;
	} else {
		this->network = new Network(original->network);
	}

	this->has_split = original->has_split;
	this->obs_index = original->obs_index;
	this->rel_obs_index = original->rel_obs_index;
	this->split_type = original->split_type;
	this->split_target = original->split_target;
	this->split_range = original->split_range;

	this->original_node_id = original->original_node_id;
	this->branch_node_id = original->branch_node_id;

	this->obs_histories = original->obs_histories;
	this->previous_val_histories = original->previous_val_histories;
	this->target_val_histories = original->target_val_histories;
}
