#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "keypoint.h"
#include "problem.h"
#include "scope.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->keypoints = vector<Keypoint*>(problem_type->num_obs(), NULL);

	this->is_used = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;
}

ObsNode::ObsNode(ObsNode* original,
				 Solution* parent_solution) {
	this->type = NODE_TYPE_OBS;

	this->is_used = original->is_used;

	for (int f_index = 0; f_index < (int)original->factors.size(); f_index++) {
		Factor* factor = new Factor(original->factors[f_index],
									parent_solution);
		this->factors.push_back(factor);
	}

	for (int k_index = 0; k_index < (int)original->keypoints.size(); k_index++) {
		if (original->keypoints[k_index] == NULL) {
			this->keypoints.push_back(NULL);
		} else {
			Keypoint* keypoint = new Keypoint(original->keypoints[k_index],
											  parent_solution);
			this->keypoints.push_back(keypoint);
		}
	}

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;
}

ObsNode::~ObsNode() {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index] != NULL) {
			delete this->keypoints[k_index];
		}
	}

	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

void ObsNode::clean_inputs(Scope* scope,
						   int node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope,
											 node_id);
	}

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index] != NULL) {
			bool should_clean = this->keypoints[k_index]->should_clean_inputs(
				scope,
				node_id);
			if (should_clean) {
				delete this->keypoints[k_index];
				this->keypoints[k_index] = NULL;
			}
		}
	}
}

void ObsNode::clean_inputs(Scope* scope) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope);
	}

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index] != NULL) {
			bool should_clean = this->keypoints[k_index]->should_clean_inputs(scope);
			if (should_clean) {
				delete this->keypoints[k_index];
				this->keypoints[k_index] = NULL;
			}
		}
	}
}

void ObsNode::clean() {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		this->keypoints[k_index]->clean();
	}

	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	this->num_measure = 0;
	this->sum_score = 0.0;
}

void ObsNode::measure_update() {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		this->keypoints[k_index]->measure_update();
	}

	this->average_score = this->sum_score / this->num_measure;
	this->average_instances_per_run = this->num_measure / MEASURE_ITERS;
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->save(output_file);
	}

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		output_file << (this->keypoints[k_index] == NULL) << endl;

		if (this->keypoints[k_index] != NULL) {
			this->keypoints[k_index]->save(output_file);
		}
	}

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average_score << endl;
	output_file << this->average_instances_per_run << endl;
}

void ObsNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		Factor* factor = new Factor();
		factor->load(input_file,
					 parent_solution);
		this->factors.push_back(factor);
	}

	for (int k_index = 0; k_index < problem_type->num_obs(); k_index++) {
		string is_null_line;
		getline(input_file, is_null_line);
		bool is_null = stoi(is_null_line);
		if (is_null) {
			this->keypoints.push_back(NULL);
		} else {
			Keypoint* keypoint = new Keypoint();
			keypoint->load(input_file,
						   parent_solution);
			this->keypoints.push_back(keypoint);
		}
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void ObsNode::link(Solution* parent_solution) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->link(parent_solution);
	}

	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}
