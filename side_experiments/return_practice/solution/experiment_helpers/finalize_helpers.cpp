#include "experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "solution.h"
#include "start_node.h"
#include "start_node.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void Experiment::add(Wrapper* wrapper) {
	stringstream ss;
	ss << get_time() << "; ";
	ss << "timestamp: " << wrapper->solution->timestamp << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		ss << " " << this->actions[s_index];
	}
	ss << "; ";

	if (this->exit_next_node == NULL) {
		ss << "this->exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->exit_next_node->id: " << this->exit_next_node->id << "; ";
	}

	ss << "this->predicted_local_improvement: " << this->predicted_local_improvement << "; ";
	ss << "this->predicted_global_improvement: " << this->predicted_global_improvement << "; ";
	ss << "this->local_improvement: " << this->local_improvement << "; ";
	ss << "this->global_improvement: " << this->global_improvement << "; ";

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)wrapper->solution->score_histories.size(); h_index++) {
		sum_vals += wrapper->solution->score_histories[h_index];
	}
	double score_average = sum_vals / (double)wrapper->solution->score_histories.size();
	cout << "score_average: " << score_average << endl;
	wrapper->solution->curr_score = score_average;

	wrapper->solution->improvement_history.push_back(score_average);
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	vector<ActionNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		ActionNode* new_action_node = new ActionNode();
		new_action_node->id = wrapper->solution->node_counter;
		wrapper->solution->node_counter++;
		wrapper->solution->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = this->actions[s_index];

		new_action_node->average_instances_per_run = this->branch_average_instances_per_run;

		new_nodes.push_back(new_action_node);
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = wrapper->solution->node_counter;
	wrapper->solution->node_counter++;
	wrapper->solution->nodes[new_branch_node->id] = new_branch_node;

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			for (int a_index = 0; a_index < (int)start_node->next_node->ancestor_ids.size(); a_index++) {
				if (start_node->next_node->ancestor_ids[a_index] == start_node->id) {
					start_node->next_node->ancestor_ids.erase(
						start_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}
			start_node->next_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->original_next_node_id = start_node->next_node_id;
			new_branch_node->original_next_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
				if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
					action_node->next_node->ancestor_ids.erase(
						action_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}
			action_node->next_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->original_next_node_id = action_node->next_node_id;
			new_branch_node->original_next_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->branch_next_node->ancestor_ids.erase(
							branch_node->branch_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				branch_node->branch_next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
				new_branch_node->original_next_node = branch_node->branch_next_node;
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->original_next_node->ancestor_ids.erase(
							branch_node->original_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				branch_node->original_next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = branch_node->original_next_node_id;
				new_branch_node->original_next_node = branch_node->original_next_node;
			}
		}
		break;
	}

	if (this->actions.size() == 0) {
		this->exit_next_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = this->exit_next_node->id;
		new_branch_node->branch_next_node = this->exit_next_node;
	} else {
		new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = new_nodes[0]->id;
		new_branch_node->branch_next_node = new_nodes[0];
	}

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			start_node->next_node_id = new_branch_node->id;
			start_node->next_node = new_branch_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			action_node->next_node_id = new_branch_node->id;
			action_node->next_node = new_branch_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				branch_node->branch_next_node_id = new_branch_node->id;
				branch_node->branch_next_node = new_branch_node;
			} else {
				branch_node->original_next_node_id = new_branch_node->id;
				branch_node->original_next_node = new_branch_node;
			}
		}
		break;
	}
	new_branch_node->ancestor_ids.push_back(this->node_context->id);

	new_branch_node->original_network = this->original_network;
	this->original_network = NULL;
	new_branch_node->branch_network = this->branch_network;
	this->branch_network = NULL;

	new_branch_node->original_average_instances_per_run = this->original_average_instances_per_run;
	new_branch_node->branch_average_instances_per_run = this->branch_average_instances_per_run;

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = this->exit_next_node->id;
			next_node = this->exit_next_node;
		} else {
			next_node_id = new_nodes[n_index+1]->id;
			next_node = new_nodes[n_index+1];
		}

		new_nodes[n_index]->next_node_id = next_node_id;
		new_nodes[n_index]->next_node = next_node;

		next_node->ancestor_ids.push_back(new_nodes[n_index]->id);
	}

	wrapper->solution->timestamp++;
}
