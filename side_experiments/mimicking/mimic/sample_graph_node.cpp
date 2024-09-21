#include "sample_graph_node.h"

using namespace std;

void SampleGraphNode::save_for_display(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_ids.size() << endl;
	for (int n_index = 0; n_index < (int)this->next_node_ids.size(); n_index++) {
		output_file << this->next_node_ids[n_index] << endl;
	}
}
