#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "obs_network.h"
#include "predict_network.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->average_instances_per_hit = 1.0;
	this->average_instances_per_run = 0.0;

	this->curr_num_instances = 0;
}

ActionNode::~ActionNode() {
	delete this->obs_network;

	delete this->predict_network;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

void ActionNode::copy_from(ActionNode* original) {
	this->is_generic = original->is_generic;

	this->action = original->action;

	this->obs_network = new ObsNetwork(original->obs_network);

	this->predict_network = new PredictNetwork(original->predict_network);

	this->next_node_id = original->next_node_id;

	this->average_instances_per_hit = original->average_instances_per_hit;
	this->average_instances_per_run = original->average_instances_per_run;

	this->ancestor_ids = original->ancestor_ids;
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->is_generic << endl;

	output_file << this->action << endl;

	this->obs_network->save(output_file);

	this->predict_network->save(output_file);

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_hit << endl;
	output_file << this->average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ActionNode::load(ifstream& input_file) {
	string is_generic_line;
	getline(input_file, is_generic_line);
	this->is_generic = stoi(is_generic_line);

	string action_line;
	getline(input_file, action_line);
	this->action = stoi(action_line);

	this->obs_network = new ObsNetwork(input_file);

	this->predict_network = new PredictNetwork(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_hit_line;
	getline(input_file, average_instances_per_hit_line);
	this->average_instances_per_hit = stod(average_instances_per_hit_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void ActionNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ActionNode::save_for_display(ofstream& output_file) {
	output_file << this->action << endl;
	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}

TrainActionNodeHistory::TrainActionNodeHistory(ActionNode* node) {
	this->node = node;
}

TrainActionNodeHistory::~TrainActionNodeHistory() {
	delete this->obs_network_history;
}

TrainPredictActionNodeHistory::TrainPredictActionNodeHistory(ActionNode* node) {
	this->node = node;
}

TrainPredictActionNodeHistory::~TrainPredictActionNodeHistory() {
	delete this->predict_network_history;
}
