#include "decision_tree.h"

#include <iostream>
#include <limits>
#include <set>

#include "decision_tree_helpers.h"
#include "linear_node.h"
#include "network.h"
#include "network_node.h"
#include "previous_node.h"

using namespace std;

const int NUM_INIT_TRIES = 10;

DecisionTree::DecisionTree() {
	this->node_counter = 0;

	this->root = NULL;

	this->history_index = 0;
}

DecisionTree::~DecisionTree() {
	for (map<int, AbstractDecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}
}

double DecisionTree::activate(vector<double>& obs) {
	if (this->root == NULL) {
		return 0.0;
	}

	AbstractDecisionTreeNode* curr_node = this->root;
	double previous_val = 0.0;
	while (true) {
		if (curr_node->has_split) {
			previous_val = curr_node->activate(obs,
											   previous_val);

			bool is_branch = is_match_helper(obs,
											 curr_node->obs_index,
											 curr_node->rel_obs_index,
											 curr_node->split_type,
											 curr_node->split_target,
											 curr_node->split_range);
			if (is_branch) {
				curr_node = curr_node->branch_node;
			} else {
				curr_node = curr_node->original_node;
			}
		} else {
			return curr_node->activate(obs,
									   previous_val);
		}
	}

	return 0.0;		// unreachable
}

void DecisionTree::backprop(vector<double>& obs,
							double target_val) {
	if (this->obs_histories.size() < DT_NUM_TOTAL_SAMPLES) {
		this->obs_histories.push_back(obs);
		this->target_val_histories.push_back(target_val);
	} else {
		this->obs_histories[this->history_index] = obs;
		this->target_val_histories[this->history_index] = target_val;

		this->history_index++;
		if (this->history_index >= DT_NUM_TOTAL_SAMPLES) {
			this->history_index = 0;
		}
	}

	if (this->root == NULL) {
		if ((int)this->obs_histories.size() >= DT_NUM_TOTAL_SAMPLES) {
			init_helper();
		}
	} else {
		AbstractDecisionTreeNode* curr_node = this->root;
		double previous_val = 0.0;
		while (true) {
			if (curr_node->has_split) {
				previous_val = curr_node->activate(obs,
												   previous_val);

				bool is_branch = is_match_helper(obs,
												 curr_node->obs_index,
												 curr_node->rel_obs_index,
												 curr_node->split_type,
												 curr_node->split_target,
												 curr_node->split_range);
				if (is_branch) {
					curr_node = curr_node->branch_node;
				} else {
					curr_node = curr_node->original_node;
				}
			} else {
				curr_node->obs_histories.push_back(obs);
				curr_node->previous_val_histories.push_back(previous_val);
				curr_node->target_val_histories.push_back(target_val);
				if (curr_node->obs_histories.size() >= DT_NUM_TOTAL_SAMPLES) {
					curr_node->update(this);
				}

				break;
			}
		}
	}
}

void DecisionTree::save(ofstream& output_file) {
	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractDecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	if (this->root == NULL) {
		output_file << -1 << endl;
	} else {
		output_file << this->root->id << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
	}
}

void DecisionTree::load(ifstream& input_file) {
	string node_counter_line;
	getline(input_file, node_counter_line);
	this->node_counter = stoi(node_counter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string id_line;
		getline(input_file, id_line);
		int id = stoi(id_line);

		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		switch (type) {
		case DECISION_TREE_NODE_TYPE_PREVIOUS:
			{
				PreviousNode* previous_node = new PreviousNode();
				previous_node->id = id;
				previous_node->load(input_file);
				this->nodes[previous_node->id] = previous_node;
			}
			break;
		case DECISION_TREE_NODE_TYPE_LINEAR:
			{
				LinearNode* linear_node = new LinearNode();
				linear_node->id = id;
				linear_node->load(input_file);
				this->nodes[linear_node->id] = linear_node;
			}
			break;
		case DECISION_TREE_NODE_TYPE_NETWORK:
			{
				NetworkNode* network_node = new NetworkNode();
				network_node->id = id;
				network_node->load(input_file);
				this->nodes[network_node->id] = network_node;
			}
			break;
		}
	}

	for (map<int, AbstractDecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(this);
	}

	string root_id_line;
	getline(input_file, root_id_line);
	int root_id = stoi(root_id_line);
	if (root_id == -1) {
		this->root = NULL;
	} else {
		this->root = this->nodes[root_id];
	}

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));
	}
}

void DecisionTree::copy_from(DecisionTree* original) {
	this->node_counter = original->node_counter;

	for (map<int, AbstractDecisionTreeNode*>::iterator it = original->nodes.begin();
			it != original->nodes.end(); it++) {
		switch (it->second->type) {
		case DECISION_TREE_NODE_TYPE_PREVIOUS:
			{
				PreviousNode* original_previous_node = (PreviousNode*)it->second;
				PreviousNode* previous_node = new PreviousNode();
				previous_node->id = it->first;
				previous_node->copy_from(original_previous_node);
				this->nodes[it->first] = previous_node;
			}
			break;
		case DECISION_TREE_NODE_TYPE_LINEAR:
			{
				LinearNode* original_linear_node = (LinearNode*)it->second;
				LinearNode* linear_node = new LinearNode();
				linear_node->id = it->first;
				linear_node->copy_from(original_linear_node);
				this->nodes[it->first] = linear_node;
			}
			break;
		case DECISION_TREE_NODE_TYPE_NETWORK:
			{
				NetworkNode* original_network_node = (NetworkNode*)it->second;
				NetworkNode* network_node = new NetworkNode();
				network_node->id = it->first;
				network_node->copy_from(original_network_node);
				this->nodes[it->first] = network_node;
			}
			break;
		}
	}

	if (original->root == NULL) {
		this->root = NULL;
	} else {
		this->root = this->nodes[original->root->id];
	}

	this->improvement_history = original->improvement_history;

	this->obs_histories = original->obs_histories;
	this->target_val_histories = original->target_val_histories;
	this->history_index = original->history_index;
}

void DecisionTree::init_helper() {
	vector<vector<double>> train_obs_histories(this->obs_histories.begin(), this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES);
	vector<double> train_previous_val_histories(DT_NUM_TRAIN_SAMPLES, 0.0);
	vector<double> train_target_val_histories(this->target_val_histories.begin(), this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES);

	vector<vector<double>> test_obs_histories(this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->obs_histories.end());
	vector<double> test_previous_val_histories(DT_NUM_TEST_SAMPLES, 0.0);
	vector<double> test_target_val_histories(this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->target_val_histories.end());

	double best_sum_misguess = numeric_limits<double>::max();

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

		if (curr_sum_misguess < best_sum_misguess) {
			best_sum_misguess = curr_sum_misguess;

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

		if (curr_sum_misguess > best_sum_misguess) {
			best_sum_misguess = curr_sum_misguess;

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

	new_node->id = this->node_counter;
	this->node_counter++;
	this->nodes[new_node->id] = new_node;

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

	this->root = new_node;

	measure_helper();
}

void DecisionTree::measure_helper() {
	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
		double predicted_val = activate(this->obs_histories[h_index]);
		sum_misguess += (this->target_val_histories[h_index] - predicted_val) * (this->target_val_histories[h_index] - predicted_val);
	}

	cout << "sum_misguess: " << sum_misguess << endl;

	this->improvement_history.push_back(sum_misguess / this->obs_histories.size());
}
