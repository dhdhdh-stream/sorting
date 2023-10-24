#include "branch_stub_node.h"

#include "globals.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchStubNode::BranchStubNode() {
	this->type = NODE_TYPE_BRANCH_STUB;

	this->id = -1;
}

BranchStubNode::BranchStubNode(ifstream& input_file,
							   int id) {
	this->type = NODE_TYPE_BRANCH_STUB;

	this->id = id;

	string was_branch_line;
	getline(input_file, was_branch_line);
	this->was_branch = stoi(was_branch_line);

	string state_defs_size_line;
	getline(input_file, state_defs_size_line);
	int state_defs_size = stoi(state_defs_size_line);
	for (int s_index = 0; s_index < state_defs_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->state_is_local.push_back(stoi(is_local_line));

		string index_line;
		getline(input_file, index_line);
		this->state_indexes.push_back(stoi(index_line));

		string def_id_line;
		getline(input_file, def_id_line);
		this->state_defs.push_back(solution->states[stoi(def_id_line)]);

		string network_index_line;
		getline(input_file, network_index_line);
		this->state_network_indexes.push_back(stoi(network_index_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

BranchStubNode::~BranchStubNode() {
	// do nothing
}

void BranchStubNode::save(ofstream& output_file) {
	output_file << this->was_branch << endl;

	output_file << this->state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_defs.size(); s_index++) {
		output_file << this->state_is_local[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
		output_file << this->state_defs[s_index]->id << endl;
		output_file << this->state_network_indexes[s_index] << endl;
	}

	output_file << this->next_node_id << endl;
}

void BranchStubNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

BranchStubNodeHistory::BranchStubNodeHistory(BranchStubNode* node) {
	this->node = node;
}
