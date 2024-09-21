#include "sample_graph.h"

#include "sample.h"
#include "sample_graph_node.h"

using namespace std;

SampleGraph::~SampleGraph() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

void SampleGraph::init(Sample* initial_sample) {
	SampleGraphNode* start_node = new SampleGraphNode();
	start_node->id = 0;
	start_node->action = Action(ACTION_NOOP);
	this->nodes.push_back(start_node);

	SampleGraphNode* end_node = new SampleGraphNode();
	end_node->id = 1;
	end_node->action = Action(ACTION_NOOP);
	this->nodes.push_back(end_node);

	SampleGraphNode* previous_node = start_node;
	for (int step_index = 0; step_index < (int)initial_sample->actions.size(); step_index++) {
		SampleGraphNode* new_node = new SampleGraphNode();
		new_node->id = this->nodes.size();

		new_node->action = initial_sample->actions[step_index];

		previous_node->next_node_ids.push_back(new_node->id);
		new_node->previous_node_ids.push_back(previous_node->id);

		this->nodes.push_back(new_node);

		previous_node = new_node;
	}

	previous_node->next_node_ids.push_back(1);
	this->nodes[1]->previous_node_ids.push_back(previous_node->id);
}

void SampleGraph::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save_for_display(output_file);
	}
}
