#include "end_node.h"

#include "experiment.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

EndNode::EndNode() {
	this->type = NODE_TYPE_END;
}

void EndNode::save(ofstream& output_file,
				   Wrapper* wrapper) {
	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void EndNode::load(ifstream& input_file,
				   Wrapper* wrapper) {
	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void EndNode::link(Wrapper* wrapper) {
	// do nothing
}

void EndNode::save_for_display(ofstream& output_file) {
	// do nothing
}

EndNodeHistory::EndNodeHistory(EndNode* node) {
	this->node = node;
}
