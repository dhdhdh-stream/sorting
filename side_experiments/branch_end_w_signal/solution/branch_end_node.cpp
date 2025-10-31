#include "branch_end_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "network.h"
#include "scope.h"

using namespace std;

BranchEndNode::BranchEndNode() {
	this->type = NODE_TYPE_BRANCH_END;

	this->pre_network = NULL;
	this->post_network = NULL;

	this->experiment = NULL;
}

BranchEndNode::~BranchEndNode() {
	if (this->pre_network != NULL) {
		delete this->pre_network;
	}

	if (this->post_network != NULL) {
		delete this->post_network;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void BranchEndNode::save(ofstream& output_file) {
	output_file << (this->pre_network == NULL) << endl;
	if (this->pre_network != NULL) {
		this->pre_network->save(output_file);
	}

	output_file << (this->post_network == NULL) << endl;
	if (this->post_network != NULL) {
		this->post_network->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->branch_start_node_id << endl;

	/**
	 * - randomly save samples for debug
	 */
	int existing_num_save = min(20, (int)this->existing_pre_histories.size());
	output_file << existing_num_save << endl;
	vector<int> existing_indexes(this->existing_pre_histories.size());
	for (int i_index = 0; i_index < (int)this->existing_pre_histories.size(); i_index++) {
		existing_indexes[i_index] = i_index;
	}
	for (int s_index = 0; s_index < existing_num_save; s_index++) {
		uniform_int_distribution<int> distribution(0, existing_indexes.size()-1);
		int index = distribution(generator);

		for (int i_index = 0; i_index < 25; i_index++) {
			output_file << this->existing_pre_histories[index][i_index] << endl;
		}

		for (int i_index = 0; i_index < 25; i_index++) {
			output_file << this->existing_post_histories[index][i_index] << endl;
		}

		output_file << this->existing_target_val_histories[index] << endl;

		existing_indexes.erase(existing_indexes.begin() + index);
	}
	int explore_num_save = min(20, (int)this->explore_pre_histories.size());
	output_file << explore_num_save << endl;
	vector<int> explore_indexes(this->explore_pre_histories.size());
	for (int i_index = 0; i_index < (int)this->explore_pre_histories.size(); i_index++) {
		explore_indexes[i_index] = i_index;
	}
	for (int s_index = 0; s_index < explore_num_save; s_index++) {
		uniform_int_distribution<int> distribution(0, explore_indexes.size()-1);
		int index = distribution(generator);

		for (int i_index = 0; i_index < 25; i_index++) {
			output_file << this->explore_pre_histories[index][i_index] << endl;
		}

		for (int i_index = 0; i_index < 25; i_index++) {
			output_file << this->explore_post_histories[index][i_index] << endl;
		}

		output_file << this->explore_target_val_histories[index] << endl;

		explore_indexes.erase(explore_indexes.begin() + index);
	}

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchEndNode::load(ifstream& input_file,
						 Solution* parent_solution) {
	string pre_network_is_null_line;
	getline(input_file, pre_network_is_null_line);
	bool pre_network_is_null = stoi(pre_network_is_null_line);
	if (!pre_network_is_null) {
		this->pre_network = new Network(input_file);
	}

	string post_network_is_null_line;
	getline(input_file, post_network_is_null_line);
	bool post_network_is_null = stoi(post_network_is_null_line);
	if (!post_network_is_null) {
		this->post_network = new Network(input_file);
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string branch_start_node_id_line;
	getline(input_file, branch_start_node_id_line);
	this->branch_start_node_id = stoi(branch_start_node_id_line);

	cout << "this->parent->id: " << this->parent->id << endl;
	cout << "this->id: " << this->id << endl;

	string existing_num_save_line;
	getline(input_file, existing_num_save_line);
	int existing_num_save = stoi(existing_num_save_line);
	for (int s_index = 0; s_index < existing_num_save; s_index++) {
		vector<double> pre_obs;
		for (int i_index = 0; i_index < 25; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			pre_obs.push_back(stod(obs_line));
		}
		this->existing_pre_histories.push_back(pre_obs);

		vector<double> post_obs;
		for (int i_index = 0; i_index < 25; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			post_obs.push_back(stod(obs_line));
		}
		this->existing_post_histories.push_back(post_obs);

		string target_val_line;
		getline(input_file, target_val_line);
		double target_val = stod(target_val_line);
		this->existing_target_val_histories.push_back(target_val);

		cout << "existing " << s_index << endl;
		cout << "pre_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << pre_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << post_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "target_val: " << target_val << endl;
		cout << endl;
	}

	string explore_num_save_line;
	getline(input_file, explore_num_save_line);
	int explore_num_save = stoi(explore_num_save_line);
	for (int s_index = 0; s_index < explore_num_save; s_index++) {
		vector<double> pre_obs;
		for (int i_index = 0; i_index < 25; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			pre_obs.push_back(stod(obs_line));
		}
		this->explore_pre_histories.push_back(pre_obs);

		vector<double> post_obs;
		for (int i_index = 0; i_index < 25; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			post_obs.push_back(stod(obs_line));
		}
		this->explore_post_histories.push_back(post_obs);

		string target_val_line;
		getline(input_file, target_val_line);
		double target_val = stod(target_val_line);
		this->explore_target_val_histories.push_back(target_val);

		cout << "explore " << s_index << endl;
		cout << "pre_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << pre_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << post_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "target_val: " << target_val << endl;
		cout << endl;
	}

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchEndNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}

	this->branch_start_node = (BranchNode*)this->parent->nodes[this->branch_start_node_id];
}

void BranchEndNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

BranchEndNodeHistory::BranchEndNodeHistory(BranchEndNode* node) {
	this->node = node;

	this->signal_sum_vals = 0.0;
	this->signal_sum_counts = 0;
}
