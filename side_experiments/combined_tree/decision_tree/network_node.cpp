#include "network_node.h"

#include "constants.h"
#include "decision_tree.h"
#include "decision_tree_helpers.h"
#include "globals.h"
#include "linear_node.h"
#include "network.h"
#include "previous_node.h"

using namespace std;

NetworkNode::NetworkNode() {
	this->type = DECISION_TREE_NODE_TYPE_NETWORK;
}

NetworkNode::~NetworkNode() {
	delete this->network;
}

double NetworkNode::activate(vector<double>& obs,
							 double previous_val) {
	vector<double> inputs(1 + this->input_indexes.size());
	inputs[0] = previous_val;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		inputs[1 + i_index] = obs[this->input_indexes[i_index]];
	}

	this->network->activate(inputs);

	return this->network->output->acti_vals[0];
}

void NetworkNode::update(DecisionTree* decision_tree) {
	vector<double> existing_predicted(this->obs_histories.size());
	for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
		existing_predicted[h_index] = activate(this->obs_histories[h_index],
											   this->previous_val_histories[h_index]);
	}

	Network* new_default_network = new Network(1 + this->input_indexes.size());

	uniform_int_distribution<int> train_distribution(0, DT_NUM_TRAIN_SAMPLES-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = train_distribution(generator);

		vector<double> inputs(1 + this->input_indexes.size());
		inputs[0] = this->previous_val_histories[index];
		for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
			inputs[1 + i_index] = this->obs_histories[index][this->input_indexes[i_index]];
		}

		new_default_network->activate(inputs);

		double error = this->target_val_histories[index] - new_default_network->output->acti_vals[0];

		new_default_network->backprop(error);
	}

	double new_default_improvement = 0.0;
	for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
		vector<double> inputs(1 + this->input_indexes.size());
		inputs[0] = this->previous_val_histories[DT_NUM_TRAIN_SAMPLES + h_index];
		for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
			inputs[1 + i_index] = this->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][this->input_indexes[i_index]];
		}

		new_default_network->activate(inputs);

		double new_misguess = (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0])
			* (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0]);
		double existing_misguess = (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index])
			* (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index]);

		new_default_improvement += (existing_misguess - new_misguess);
	}

	if (new_default_improvement > 0.0) {
		delete this->network;
		this->network = new_default_network;
	} else {
		delete new_default_network;

		vector<vector<double>> train_obs_histories(this->obs_histories.begin(), this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES);
		vector<double> train_previous_val_histories(existing_predicted.begin(), existing_predicted.begin() + DT_NUM_TRAIN_SAMPLES);
		vector<double> train_target_val_histories(this->target_val_histories.begin(), this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES);

		vector<vector<double>> test_obs_histories(this->obs_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->obs_histories.end());
		vector<double> test_previous_val_histories(existing_predicted.begin() + DT_NUM_TRAIN_SAMPLES, existing_predicted.end());
		vector<double> test_target_val_histories(this->target_val_histories.begin() + DT_NUM_TRAIN_SAMPLES, this->target_val_histories.end());

		double best_improvement = 0.0;

		int best_obs_index;
		int best_rel_obs_index;
		int best_split_type;
		double best_split_target;
		double best_split_range;

		int best_type;

		vector<int> best_input_indexes;

		double best_constant;
		vector<double> best_input_weights;
		double best_previous_weight;

		Network* best_network = NULL;

		for (int split_index = 0; split_index < SPLIT_NUM_TRIES; split_index++) {
			int curr_obs_index;
			int curr_rel_obs_index;
			int curr_split_type;
			double curr_split_target;
			double curr_split_range;
			split_try(train_obs_histories,
					  train_target_val_histories,
					  curr_obs_index,
					  curr_rel_obs_index,
					  curr_split_type,
					  curr_split_target,
					  curr_split_range);

			vector<vector<double>> match_train_obs_histories;
			vector<double> match_train_previous_val_histories;
			vector<double> match_train_target_val_histories;
			for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
				bool is_match = is_match_helper(train_obs_histories[h_index],
												curr_obs_index,
												curr_rel_obs_index,
												curr_split_type,
												curr_split_target,
												curr_split_range);
				if (is_match) {
					match_train_obs_histories.push_back(train_obs_histories[h_index]);
					match_train_previous_val_histories.push_back(train_previous_val_histories[h_index]);
					match_train_target_val_histories.push_back(train_target_val_histories[h_index]);
				}
			}

			vector<vector<double>> match_test_obs_histories;
			vector<double> match_test_previous_val_histories;
			vector<double> match_test_target_val_histories;
			double match_existing_sum_misguess = 0.0;
			for (int h_index = 0; h_index < (int)test_obs_histories.size(); h_index++) {
				bool is_match = is_match_helper(test_obs_histories[h_index],
												curr_obs_index,
												curr_rel_obs_index,
												curr_split_type,
												curr_split_target,
												curr_split_range);
				if (is_match) {
					match_test_obs_histories.push_back(test_obs_histories[h_index]);
					match_test_previous_val_histories.push_back(test_previous_val_histories[h_index]);
					match_test_target_val_histories.push_back(test_target_val_histories[h_index]);

					match_existing_sum_misguess += (test_target_val_histories[h_index] - test_previous_val_histories[h_index])
						* (test_target_val_histories[h_index] - test_previous_val_histories[h_index]);
				}
			}

			for (int try_index = 0; try_index < LINEAR_TRIES_PER; try_index++) {
				double curr_constant;
				vector<int> curr_input_indexes;
				vector<double> curr_input_weights;
				double curr_previous_weight;
				double curr_sum_misguess;
				linear_try(match_train_obs_histories,
						   match_train_previous_val_histories,
						   match_train_target_val_histories,
						   curr_constant,
						   curr_input_indexes,
						   curr_input_weights,
						   curr_previous_weight,
						   match_test_obs_histories,
						   match_test_previous_val_histories,
						   match_test_target_val_histories,
						   curr_sum_misguess);

				double curr_improvement = match_existing_sum_misguess - curr_sum_misguess;
				if (curr_improvement > best_improvement) {
					best_improvement = curr_improvement;

					best_obs_index = curr_obs_index;
					best_rel_obs_index = curr_rel_obs_index;
					best_split_type = curr_split_type;
					best_split_target = curr_split_target;
					best_split_range = curr_split_range;

					best_type = DECISION_TREE_NODE_TYPE_LINEAR;

					best_input_indexes = curr_input_indexes;

					best_constant = curr_constant;
					best_input_weights = curr_input_weights;
					best_previous_weight = curr_previous_weight;

					if (best_network != NULL) {
						delete best_network;
					}
					best_network = NULL;
				}
			}

			{
				vector<int> curr_input_indexes;
				Network* curr_network = NULL;
				double curr_sum_misguess;
				network_try(match_train_obs_histories,
							match_train_previous_val_histories,
							match_train_target_val_histories,
							curr_input_indexes,
							curr_network,
							match_test_obs_histories,
							match_test_previous_val_histories,
							match_test_target_val_histories,
							curr_sum_misguess);

				double curr_improvement = match_existing_sum_misguess - curr_sum_misguess;
				if (curr_improvement > best_improvement) {
					best_improvement = curr_improvement;

					best_obs_index = curr_obs_index;
					best_rel_obs_index = curr_rel_obs_index;
					best_split_type = curr_split_type;
					best_split_target = curr_split_target;
					best_split_range = curr_split_range;

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
		}

		if (best_improvement > 0.0) {
			AbstractDecisionTreeNode* new_match_node;
			switch (best_type) {
			case DECISION_TREE_NODE_TYPE_LINEAR:
				{
					LinearNode* new_linear_node = new LinearNode();

					new_linear_node->constant = best_constant;
					new_linear_node->input_indexes = best_input_indexes;
					new_linear_node->input_weights = best_input_weights;
					new_linear_node->previous_weight = best_previous_weight;

					new_match_node = new_linear_node;
				}
				break;
			case DECISION_TREE_NODE_TYPE_NETWORK:
				{
					NetworkNode* new_network_node = new NetworkNode();

					new_network_node->input_indexes = best_input_indexes;
					new_network_node->network = best_network;

					new_match_node = new_network_node;
				}
				break;
			}

			new_match_node->id = decision_tree->node_counter;
			decision_tree->node_counter++;
			decision_tree->nodes[new_match_node->id] = new_match_node;

			new_match_node->has_split = false;
			new_match_node->obs_index = -1;
			new_match_node->rel_obs_index = -1;
			new_match_node->split_type = -1;
			new_match_node->split_target = 0.0;
			new_match_node->split_range = 0.0;

			new_match_node->original_node_id = -1;
			new_match_node->original_node = NULL;
			new_match_node->branch_node_id = -1;
			new_match_node->branch_node = NULL;

			PreviousNode* new_non_match_node = new PreviousNode();
			new_non_match_node->id = decision_tree->node_counter;
			decision_tree->node_counter++;
			decision_tree->nodes[new_non_match_node->id] = new_non_match_node;

			new_non_match_node->has_split = false;
			new_non_match_node->obs_index = -1;
			new_non_match_node->rel_obs_index = -1;
			new_non_match_node->split_type = -1;
			new_non_match_node->split_target = 0.0;
			new_non_match_node->split_range = 0.0;

			new_non_match_node->original_node_id = -1;
			new_non_match_node->original_node = NULL;
			new_non_match_node->branch_node_id = -1;
			new_non_match_node->branch_node = NULL;

			this->has_split = true;
			this->obs_index = best_obs_index;
			this->rel_obs_index = best_rel_obs_index;
			this->split_type = best_split_type;
			this->split_target = best_split_target;
			this->split_range = best_split_range;

			this->original_node_id = new_non_match_node->id;
			this->original_node = new_non_match_node;
			this->branch_node_id = new_match_node->id;
			this->branch_node = new_match_node;
		}
	}

	this->obs_histories.clear();
	this->previous_val_histories.clear();
	this->target_val_histories.clear();

	decision_tree->measure_helper();
}

void NetworkNode::save(ofstream& output_file) {
	output_file << this->input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		output_file << this->input_indexes[i_index] << endl;
	}
	this->network->save(output_file);

	output_file << this->has_split << endl;
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;
}

void NetworkNode::load(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->input_indexes.push_back(stoi(index_line));
	}

	this->network = new Network(input_file);

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

void NetworkNode::link(DecisionTree* decision_tree) {
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

void NetworkNode::copy_from(NetworkNode* original) {
	this->input_indexes = original->input_indexes;
	this->network = new Network(original->network);

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
