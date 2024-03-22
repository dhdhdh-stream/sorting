#include "action_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;
}

ActionNode::~ActionNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

void ActionNode::reset() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
	this->experiments.clear();
}

void ActionNode::save(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_id << endl;
}

void ActionNode::load(ifstream& input_file) {
	this->action = Action(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ActionNode::link() {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ActionNode::save_for_display(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->node = original->node;

	this->obs_snapshot = original->obs_snapshot;
}
