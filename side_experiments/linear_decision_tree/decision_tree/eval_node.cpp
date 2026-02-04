#include "eval_node.h"

#include "network.h"

using namespace std;

EvalNode::EvalNode() {
	this->type = DECISION_TREE_NODE_TYPE_EVAL;
}

double EvalNode::activate(std::vector<double>& obs) {
	double sum_vals = constant;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		sum_vals += this->input_weights[i_index] * obs[this->input_indexes[i_index]];
	}

	return sum_vals;
}

void EvalNode::save(ofstream& output_file) {
	output_file << this->constant << endl;
	output_file << this->input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		output_file << this->input_indexes[i_index] << endl;
		output_file << this->input_weights[i_index] << endl;
	}
}

void EvalNode::load(ifstream& input_file) {
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
}

void EvalNode::copy_from(EvalNode* original) {
	this->constant = original->constant;
	this->input_indexes = original->input_indexes;
	this->input_weights = original->input_weights;

	this->obs_histories = original->obs_histories;
	this->target_val_histories = original->target_val_histories;
}
