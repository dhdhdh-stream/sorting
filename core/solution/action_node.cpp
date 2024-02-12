#include "action_node.h"

#include "branch_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->experiment = NULL;
}

ActionNode::~ActionNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ActionNode::success_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void ActionNode::fail_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
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

	this->experiment_history = NULL;
}

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->node = original->node;

	this->obs_snapshot = original->obs_snapshot;

	if (original->experiment_history != NULL) {
		if (original->experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new BranchExperimentInstanceHistory(branch_experiment_history);
		} else {
			PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new PassThroughExperimentInstanceHistory(pass_through_experiment_history);
		}
	} else {
		this->experiment_history = NULL;
	}
}

ActionNodeHistory::~ActionNodeHistory() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}

