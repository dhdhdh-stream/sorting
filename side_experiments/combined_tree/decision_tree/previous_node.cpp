#include "previous_node.h"

#include "decision_tree.h"
#include "decision_tree_helpers.h"
#include "linear_node.h"
#include "network.h"
#include "network_node.h"

using namespace std;

PreviousNode::PreviousNode() {
	this->type = DECISION_TREE_NODE_TYPE_PREVIOUS;
}

double PreviousNode::activate(vector<double>& obs,
							  double previous_val) {
	return previous_val;
}

void PreviousNode::update(DecisionTree* decision_tree) {
	double existing_sum_misguess = 0.0;
	for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
		double val = activate(this->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index],
							  this->previous_val_histories[DT_NUM_TRAIN_SAMPLES + h_index]);

		existing_sum_misguess += (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - val)
			* (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - val);
	}

	vector<vector<double>> train_obs_histories(this->obs_histories.begin(), this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES);
	vector<double> train_previous_val_histories(this->previous_val_histories.begin(), this->previous_val_histories.begin() + DT_NUM_TRAIN_SAMPLES);
	vector<double> train_target_val_histories(this->target_val_histories.begin(), this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES);

	vector<vector<double>> test_obs_histories(this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->obs_histories.end());
	vector<double> test_previous_val_histories(this->previous_val_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->previous_val_histories.end());
	vector<double> test_target_val_histories(this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->target_val_histories.end());

	double best_improvement = 0.0;

	int best_type;

	vector<int> best_input_indexes;

	double best_constant;
	vector<double> best_input_weights;
	double best_previous_weight;

	Network* best_network = NULL;

	for (int try_index = 0; try_index < LINEAR_TRIES_PER; try_index++) {
		double curr_constant;
		vector<int> curr_input_indexes;
		vector<double> curr_input_weights;
		double curr_previous_weight;
		double curr_sum_misguess;
		linear_try(train_obs_histories,
				   train_previous_val_histories,
				   train_target_val_histories,
				   curr_constant,
				   curr_input_indexes,
				   curr_input_weights,
				   curr_previous_weight,
				   test_obs_histories,
				   test_previous_val_histories,
				   test_target_val_histories,
				   curr_sum_misguess);

		double curr_improvement = existing_sum_misguess - curr_sum_misguess;
		if (curr_improvement > best_improvement) {
			best_improvement = curr_improvement;

			best_type = DECISION_TREE_NODE_TYPE_LINEAR;

			best_input_indexes = curr_input_indexes;

			best_constant = curr_constant;
			best_input_weights = curr_input_weights;
			best_previous_weight = curr_previous_weight;
		}
	}

	{
		vector<int> curr_input_indexes;
		Network* curr_network = NULL;
		double curr_sum_misguess;
		network_try(train_obs_histories,
					train_previous_val_histories,
					train_target_val_histories,
					curr_input_indexes,
					curr_network,
					test_obs_histories,
					test_previous_val_histories,
					test_target_val_histories,
					curr_sum_misguess);

		double curr_improvement = existing_sum_misguess - curr_sum_misguess;
		if (curr_improvement > best_improvement) {
			best_improvement = curr_improvement;

			best_type = DECISION_TREE_NODE_TYPE_NETWORK;

			best_input_indexes = curr_input_indexes;

			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_network;
		} else {
			if (curr_network != NULL) {
				delete curr_network;
			}
		}
	}

	this->obs_histories.clear();
	this->previous_val_histories.clear();
	this->target_val_histories.clear();

	if (best_improvement > 0.0) {
		AbstractDecisionTreeNode* parent;
		bool is_branch;
		for (map<int, AbstractDecisionTreeNode*>::iterator it = decision_tree->nodes.begin();
				it != decision_tree->nodes.end(); it++) {
			if (it->second->original_node == this) {
				parent = it->second;
				is_branch = false;
				break;
			}
			if (it->second->branch_node == this) {
				parent = it->second;
				is_branch = true;
				break;
			}
		}

		AbstractDecisionTreeNode* new_node;
		switch (best_type) {
		case DECISION_TREE_NODE_TYPE_LINEAR:
			{
				LinearNode* new_linear_node = new LinearNode();

				new_linear_node->constant = best_constant;
				new_linear_node->input_indexes = best_input_indexes;
				new_linear_node->input_weights = best_input_weights;
				new_linear_node->previous_weight = best_previous_weight;

				new_node = new_linear_node;
			}
			break;
		case DECISION_TREE_NODE_TYPE_NETWORK:
			{
				NetworkNode* new_network_node = new NetworkNode();

				new_network_node->input_indexes = best_input_indexes;
				new_network_node->network = best_network;

				new_node = new_network_node;
			}
			break;
		}

		new_node->id = decision_tree->node_counter;
		decision_tree->node_counter++;
		decision_tree->nodes[new_node->id] = new_node;

		new_node->has_split = false;
		new_node->obs_index = -1;
		new_node->rel_obs_index = -1;
		new_node->split_type = -1;
		new_node->split_target = 0.0;
		new_node->split_range = 0.0;

		new_node->original_node_id = -1;
		new_node->original_node = NULL;
		new_node->branch_node_id = -1;
		new_node->branch_node = NULL;

		if (is_branch) {
			parent->branch_node_id = new_node->id;
			parent->branch_node = new_node;
		} else {
			parent->original_node_id = new_node->id;
			parent->original_node = new_node;
		}

		decision_tree->nodes.erase(this->id);
		delete this;
	}

	decision_tree->measure_helper();
}

void PreviousNode::save(ofstream& output_file) {
	output_file << this->has_split << endl;
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;
}

void PreviousNode::load(ifstream& input_file) {
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

void PreviousNode::link(DecisionTree* decision_tree) {
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

void PreviousNode::copy_from(PreviousNode* original) {
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
