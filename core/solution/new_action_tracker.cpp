#include "new_action_tracker.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

const vector<double> EPOCH_UNTIL_RATIOS{
	1.0,
	0.5,
	0.25,
	0.0
};

NewActionNodeTracker::NewActionNodeTracker(bool is_branch,
										   AbstractNode* exit_next_node) {
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->existing_score = 0.0;
	this->existing_count = 0;
	this->new_score = 0.0;
	this->new_count = 0;
}

NewActionTracker::NewActionTracker() {
	// do nothing
}

NewActionTracker::~NewActionTracker() {
	for (map<AbstractNode*, NewActionNodeTracker*>::iterator it = this->node_trackers.begin();
			it != this->node_trackers.end(); it++) {
		delete it->second;
	}
}

NewActionTracker::NewActionTracker(NewActionTracker* original,
								   Solution* parent_solution) {
	this->epoch_iter = original->epoch_iter;
	this->improvement_iter = original->improvement_iter;

	for (map<AbstractNode*, NewActionNodeTracker*>::iterator it = original->node_trackers.begin();
			it != original->node_trackers.end(); it++) {
		AbstractNode* start_node = parent_solution->current->nodes[it->first->id];
		if (it->second->exit_next_node == NULL) {
			this->node_trackers[start_node] = new NewActionNodeTracker(
				it->second->is_branch,
				NULL);
		} else {
			this->node_trackers[start_node] = new NewActionNodeTracker(
				it->second->is_branch,
				parent_solution->current->nodes[it->second->exit_next_node->id]);
		}

		this->node_trackers[start_node]->existing_score = it->second->existing_score;
		this->node_trackers[start_node]->existing_count = it->second->existing_count;
		this->node_trackers[start_node]->new_score = it->second->new_score;
		this->node_trackers[start_node]->new_count = it->second->new_count;
	}

	this->num_actions_until_distribution = original->num_actions_until_distribution;

	parent_solution->num_actions_until_experiment = 1 + this->num_actions_until_distribution(generator);
}

void NewActionTracker::increment() {
	this->improvement_iter++;
	if (this->improvement_iter >= NEW_ACTION_IMPROVEMENTS_PER_EPOCH) {
		this->improvement_iter = 0;
		this->epoch_iter++;

		for (int f_index = 0; f_index < NEW_ACTION_MAX_FILTER_PER_EPOCH; f_index++) {
			double max_impact = 0.0;
			AbstractNode* max_node = NULL;
			for (map<AbstractNode*, NewActionNodeTracker*>::iterator it = this->node_trackers.begin();
					it != this->node_trackers.end(); it++) {
				double existing_average_score = it->second->existing_score / it->second->existing_count;
				double new_average_score = it->second->new_score / it->second->new_count;
				double impact = existing_average_score - new_average_score;
				if (impact > max_impact) {
					max_impact = impact;
					max_node = it->first;
				}
			}

			if (max_node != NULL) {
				delete this->node_trackers[max_node];
				this->node_trackers.erase(max_node);
			} else {
				break;
			}
		}

		this->num_actions_until_distribution = uniform_int_distribution<int>(0,
			(int)(EPOCH_UNTIL_RATIOS[this->epoch_iter] * solution->average_num_actions));
	}
}

void NewActionTracker::init(Solution* parent_solution) {
	this->epoch_iter = 0;
	this->improvement_iter = 0;

	this->num_actions_until_distribution = uniform_int_distribution<int>(0, (int)parent_solution->average_num_actions);

	parent_solution->num_actions_until_experiment = 1 + this->num_actions_until_distribution(generator);
}

void NewActionTracker::load(ifstream& input_file) {
	string epoch_iter_line;
	getline(input_file, epoch_iter_line);
	this->epoch_iter = stoi(epoch_iter_line);

	string improvement_iter_line;
	getline(input_file, improvement_iter_line);
	this->improvement_iter = stoi(improvement_iter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string start_node_id_line;
		getline(input_file, start_node_id_line);
		AbstractNode* starting_node = solution->current->nodes[stoi(start_node_id_line)];

		string is_branch_line;
		getline(input_file, is_branch_line);
		bool is_branch = stoi(is_branch_line);

		string end_node_id_line;
		getline(input_file, end_node_id_line);
		int end_node_id = stoi(end_node_id_line);
		if (end_node_id == -1) {
			this->node_trackers[starting_node] = new NewActionNodeTracker(
				is_branch,
				NULL);
		} else {
			this->node_trackers[starting_node] = new NewActionNodeTracker(
				is_branch,
				solution->current->nodes[end_node_id]);
		}
	}

	this->num_actions_until_distribution = uniform_int_distribution<int>(0,
		(int)(EPOCH_UNTIL_RATIOS[this->epoch_iter] * solution->average_num_actions));

	solution->num_actions_until_experiment = 1 + this->num_actions_until_distribution(generator);
}

void NewActionTracker::save(ofstream& output_file) {
	output_file << this->epoch_iter << endl;
	output_file << this->improvement_iter << endl;

	output_file << this->node_trackers.size() << endl;
	for (map<AbstractNode*, NewActionNodeTracker*>::iterator it = this->node_trackers.begin();
			it != this->node_trackers.end(); it++) {
		output_file << it->first->id << endl;

		output_file << it->second->is_branch << endl;

		if (it->second->exit_next_node == NULL) {
			output_file << -1 << endl;
		} else {
			output_file << it->second->exit_next_node->id << endl;
		}
	}
}

NewActionHistory::NewActionHistory() {
	// do nothing
}
