#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->network = NULL;

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->input_types = original->input_types;
	this->input_locations = original->input_locations;
	this->input_node_context_ids = original->input_node_context_ids;
	this->input_obs_indexes = original->input_obs_indexes;
	this->network = new Network(original->network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	if (this->network != NULL) {
		delete this->network;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::save(ofstream& output_file) {
	output_file << this->input_types.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		output_file << this->input_types[i_index] << endl;

		output_file << this->input_locations[i_index].size() << endl;
		for (int l_index = 0; l_index < (int)this->input_locations[i_index].size(); l_index++) {
			output_file << this->input_locations[i_index][l_index] << endl;
		}

		output_file << this->input_node_context_ids[i_index] << endl;

		output_file << this->input_obs_indexes[i_index] << endl;
	}

	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void BranchNode::load(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string type_line;
		getline(input_file, type_line);
		this->input_types.push_back(stoi(type_line));

		string location_size_line;
		getline(input_file, location_size_line);
		int location_size = stoi(location_size_line);
		vector<int> location;
		for (int l_index = 0; l_index < location_size; l_index++) {
			string component_line;
			getline(input_file, component_line);
			location.push_back(stoi(component_line));
		}
		this->input_locations.push_back(location);

		string node_context_id_line;
		getline(input_file, node_context_id_line);
		this->input_node_context_ids.push_back(stoi(node_context_id_line));

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}

	this->network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void BranchNode::link(Solution* parent_solution) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_node_context_ids[i_index] == -1) {
			this->input_node_contexts.push_back(NULL);
		} else {
			this->input_node_contexts.push_back(this->parent->nodes[this->input_node_context_ids[i_index]]);
		}
	}

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
