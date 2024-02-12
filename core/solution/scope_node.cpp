#include "scope_node.h"

#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->id = -1;

	this->experiment = NULL;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::success_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void ScopeNode::fail_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

void ScopeNode::load(ifstream& input_file) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = solution->scopes[stoi(scope_id_line)];

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ScopeNode::link() {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->experiment_history = NULL;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;

	this->scope_history = new ScopeHistory(original->scope_history);

	if (original->experiment_history != NULL) {
		if (original->experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new BranchExperimentInstanceHistory(branch_experiment_history);
		} else {
			// original->experiment_history->experiment->type == EXPERIMENT_TYPE_PASS_THROUGH
			PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new PassThroughExperimentInstanceHistory(pass_through_experiment_history);
		}
	} else {
		this->experiment_history = NULL;
	}
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;

	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}
