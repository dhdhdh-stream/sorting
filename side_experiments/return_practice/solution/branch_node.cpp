#include "branch_node.h"

#include "experiment.h"
#include "network.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->original_network_epoch_iter = 0;
	this->branch_network_epoch_iter = 0;

	this->original_state_history_index = 0;
	this->branch_state_history_index = 0;

	/**
	 * - simply initialize to 1.0
	 */
	this->original_average_instances_per_run = 1.0;
	this->branch_average_instances_per_run = 1.0;

	this->original_experiment = NULL;
	this->branch_experiment = NULL;

	this->original_sum_instances = 0;
	this->branch_sum_instances = 0;
}

BranchNode::~BranchNode() {
	delete this->original_network;
	delete this->branch_network;

	if (this->original_experiment != NULL) {
		delete this->original_experiment;
	}
	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
}

void BranchNode::save(ofstream& output_file,
					  Wrapper* wrapper) {
	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->original_state_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->original_state_history.size(); h_index++) {
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			output_file << this->original_state_history[h_index][s_index] << endl;
		}
	}
	output_file << this->original_state_history_index << endl;

	output_file << this->branch_state_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->branch_state_history.size(); h_index++) {
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			output_file << this->branch_state_history[h_index][s_index] << endl;
		}
	}
	output_file << this->branch_state_history_index << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Wrapper* wrapper) {
	this->original_network = new Network(input_file);
	this->branch_network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string num_original_state_history_line;
	getline(input_file, num_original_state_history_line);
	int num_original_state_history = stoi(num_original_state_history_line);
	for (int h_index = 0; h_index < num_original_state_history; h_index++) {
		vector<double> states;
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			string state_line;
			getline(input_file, state_line);
			states.push_back(stod(state_line));
		}
		this->original_state_history.push_back(states);
	}

	string original_state_history_index_line;
	getline(input_file, original_state_history_index_line);
	this->original_state_history_index = stoi(original_state_history_index_line);

	string num_branch_state_history_line;
	getline(input_file, num_branch_state_history_line);
	int num_branch_state_history = stoi(num_branch_state_history_line);
	for (int h_index = 0; h_index < num_branch_state_history; h_index++) {
		vector<double> states;
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			string state_line;
			getline(input_file, state_line);
			states.push_back(stod(state_line));
		}
		this->branch_state_history.push_back(states);
	}

	string branch_state_history_index_line;
	getline(input_file, branch_state_history_index_line);
	this->branch_state_history_index = stoi(branch_state_history_index_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchNode::link(Wrapper* wrapper) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = wrapper->solution->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = wrapper->solution->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
