#include "action_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution.h"
#include "state.h"
#include "try_tracker.h"

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
	this->temp_state_scope_contexts.clear();
	this->temp_state_node_contexts.clear();
	this->temp_state_obs_indexes.clear();
	this->temp_state_defs.clear();
	this->temp_state_network_indexes.clear();

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

	output_file << this->state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_defs.size(); s_index++) {
		output_file << this->state_is_local[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
		output_file << this->state_obs_indexes[s_index] << endl;
		output_file << this->state_defs[s_index]->id << endl;
		output_file << this->state_network_indexes[s_index] << endl;
	}

	output_file << this->next_node_id << endl;

	output_file << this->tries.size() << endl;
	for (map<pair<vector<int>,vector<int>>, TryTracker*>::iterator it = this->tries.begin();
			it != this->tries.end(); it++) {
		output_file << it->first.first.size() << endl;
		for (int l_index = 0; l_index < (int)it->first.first.size(); l_index++) {
			output_file << it->first.first[l_index] << endl;
			output_file << it->first.second[l_index] << endl;
		}

		it->second->save(output_file);
	}
}

void ActionNode::load(ifstream& input_file) {
	this->action = Action(input_file);

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

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->state_obs_indexes.push_back(stoi(obs_index_line));

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

	string num_tries_contexts_line;
	getline(input_file, num_tries_contexts_line);
	int num_tries_contexts = stoi(num_tries_contexts_line);
	for (int c_index = 0; c_index < num_tries_contexts; c_index++) {
		string context_size_line;
		getline(input_file, context_size_line);
		int context_size = stoi(context_size_line);
		vector<int> scope_context;
		vector<int> node_context;
		for (int l_index = 0; l_index < context_size; l_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			scope_context.push_back(stoi(scope_id_line));

			string node_id_line;
			getline(input_file, node_id_line);
			node_context.push_back(stoi(node_id_line));
		}

		TryTracker* try_tracker = new TryTracker(input_file);

		this->tries[{scope_context, node_context}] = try_tracker;
	}
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

	this->state_snapshots = original->state_snapshots;

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
