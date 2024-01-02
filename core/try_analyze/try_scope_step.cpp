#include "try_scope_step.h"

using namespace std;

TryScopeStep::TryScopeStep() {
	// do nothing
}

TryScopeStep::TryScopeStep(ifstream& input_file) {
	string original_nodes_size_line;
	getline(input_file, original_nodes_size_line);
	int original_nodes_size = stoi(original_nodes_size_line);
	for (int n_index = 0; n_index < original_nodes_size; n_index++) {
		string parent_id_line;
		getline(input_file, parent_id_line);
		int parent_id = stoi(parent_id_line);

		string node_id_line;
		getline(input_file, node_id_line);
		int node_id = stoi(node_id_line);

		this->original_nodes.push_back({parent_id, node_id});
	}
}

void TryScopeStep::save(ofstream& output_file) {
	output_file << this->original_nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->original_nodes.size(); n_index++) {
		output_file << this->original_nodes[n_index].first << endl;
		output_file << this->original_nodes[n_index].second << endl;
	}
}
