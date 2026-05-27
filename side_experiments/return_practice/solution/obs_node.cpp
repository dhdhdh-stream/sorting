#include "obs_node.h"

#include "experiment.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->state_history_index = 0;

	/**
	 * - simply initialize to 1.0
	 */
	this->average_instances_per_run = 1.0;

	this->experiment = NULL;

	this->sum_instances = 0;
}

ObsNode::~ObsNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ObsNode::save(ofstream& output_file,
				   Wrapper* wrapper) {
	output_file << this->next_node_id << endl;

	output_file << this->state_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->state_history.size(); h_index++) {
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			output_file << this->state_history[h_index][s_index] << endl;
		}
	}
	output_file << this->state_history_index << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ObsNode::load(ifstream& input_file,
				   Wrapper* wrapper) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_state_history_line;
	getline(input_file, num_state_history_line);
	int num_state_history = stoi(num_state_history_line);
	for (int h_index = 0; h_index < num_state_history; h_index++) {
		vector<double> states;
		for (int s_index = 0; s_index < wrapper->world_model->num_states; s_index++) {
			string state_line;
			getline(input_file, state_line);
			states.push_back(stod(state_line));
		}
		this->state_history.push_back(states);
	}

	string state_history_index_line;
	getline(input_file, state_history_index_line);
	this->state_history_index = stoi(state_history_index_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void ObsNode::link(Wrapper* wrapper) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = wrapper->solution->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}
