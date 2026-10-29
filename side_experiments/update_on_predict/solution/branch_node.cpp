#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "predict_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->original_curr_num_instances = 0;
	this->branch_curr_num_instances = 0;
}

BranchNode::~BranchNode() {
	delete this->original_network;
	delete this->branch_network;

	delete this->branch_predict_network;

	for (int e_index = 0; e_index < (int)this->original_experiments.size(); e_index++) {
		delete this->original_experiments[e_index];
	}
	for (int e_index = 0; e_index < (int)this->branch_experiments.size(); e_index++) {
		delete this->branch_experiments[e_index];
	}
}

void BranchNode::copy_from(BranchNode* original,
						   Solution* parent_solution) {
	this->original_network = new Network(original->original_network);
	this->branch_network = new Network(original->branch_network);

	this->branch_predict_network = new ScoreNetwork(original->branch_predict_network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ramp = original->ramp;
	this->existing_sum_score = original->existing_sum_score;
	this->existing_count = original->existing_count;
	this->new_sum_score = original->new_sum_score;
	this->new_count = original->new_count;

	this->consec_original = original->consec_original;
	this->consec_branch = original->consec_branch;

	this->original_average_instances_per_hit = original->original_average_instances_per_hit;
	this->original_average_instances_per_run = original->original_average_instances_per_run;
	this->original_val_average = original->original_val_average;
	this->branch_average_instances_per_hit = original->branch_average_instances_per_hit;
	this->branch_average_instances_per_run = original->branch_average_instances_per_run;
	this->branch_val_average = original->branch_val_average;

	this->predict_rate = original->predict_rate;

	this->ancestor_ids = original->ancestor_ids;
}

void BranchNode::save(ofstream& output_file) {
	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	this->branch_predict_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ramp << endl;
	output_file << this->existing_sum_score << endl;
	output_file << this->existing_count << endl;
	output_file << this->new_sum_score << endl;
	output_file << this->new_count << endl;

	output_file << this->consec_original << endl;
	output_file << this->consec_branch << endl;

	output_file << this->original_average_instances_per_hit << endl;
	output_file << this->original_average_instances_per_run << endl;
	output_file << this->original_val_average << endl;
	output_file << this->branch_average_instances_per_hit << endl;
	output_file << this->branch_average_instances_per_run << endl;
	output_file << this->branch_val_average << endl;

	output_file << this->predict_rate << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	this->original_network = new Network(input_file);
	this->branch_network = new Network(input_file);

	this->branch_predict_network = new ScoreNetwork(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string ramp_line;
	getline(input_file, ramp_line);
	this->ramp = stoi(ramp_line);

	string existing_sum_score_line;
	getline(input_file, existing_sum_score_line);
	this->existing_sum_score = stod(existing_sum_score_line);

	string existing_count_line;
	getline(input_file, existing_count_line);
	this->existing_count = stoi(existing_count_line);

	string new_sum_score_line;
	getline(input_file, new_sum_score_line);
	this->new_sum_score = stod(new_sum_score_line);

	string new_count_line;
	getline(input_file, new_count_line);
	this->new_count = stoi(new_count_line);

	string consec_original_line;
	getline(input_file, consec_original_line);
	this->consec_original = stoi(consec_original_line);

	// temp
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_original >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string consec_branch_line;
	getline(input_file, consec_branch_line);
	this->consec_branch = stoi(consec_branch_line);

	// temp
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_branch >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string original_average_instances_per_hit_line;
	getline(input_file, original_average_instances_per_hit_line);
	this->original_average_instances_per_hit = stod(original_average_instances_per_hit_line);

	string original_average_instances_per_run_line;
	getline(input_file, original_average_instances_per_run_line);
	this->original_average_instances_per_run = stod(original_average_instances_per_run_line);

	string original_val_average_line;
	getline(input_file, original_val_average_line);
	this->original_val_average = stod(original_val_average_line);

	string branch_average_instances_per_hit_line;
	getline(input_file, branch_average_instances_per_hit_line);
	this->branch_average_instances_per_hit = stod(branch_average_instances_per_hit_line);

	string branch_average_instances_per_run_line;
	getline(input_file, branch_average_instances_per_run_line);
	this->branch_average_instances_per_run = stod(branch_average_instances_per_run_line);

	string branch_val_average_line;
	getline(input_file, branch_val_average_line);
	this->branch_val_average = stod(branch_val_average_line);

	string predict_rate_line;
	getline(input_file, predict_rate_line);
	this->predict_rate = stod(predict_rate_line);

	// temp
	cout << "this->parent->id: " << this->parent->id << endl;
	cout << "this->id: " << this->id << endl;
	cout << "this->predict_rate: " << this->predict_rate << endl;

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchNode::link(Solution* parent_solution) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}

TrainPredictBranchNodeHistory::TrainPredictBranchNodeHistory(BranchNode* node) {
	this->node = node;
}
