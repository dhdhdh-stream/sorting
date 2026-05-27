#include "experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "solution.h"
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

		new_nodes.push_back(new_action_node);
	}

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		new_ending_node = new ObsNode();
		new_ending_node->id = wrapper->solution->node_counter;
		wrapper->solution->node_counter++;

		for (map<int, AbstractNode*>::iterator it = wrapper->solution->nodes.begin();
				it != wrapper->solution->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					obs_node->next_node_id = new_ending_node->id;
					obs_node->next_node = new_ending_node;

					new_ending_node->ancestor_ids.push_back(obs_node->id);

					break;
				}
			}
		}

		wrapper->solution->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = wrapper->solution->node_counter;
	wrapper->solution->node_counter++;
	wrapper->solution->nodes[new_branch_node->id] = new_branch_node;

	switch (this->node_context->type) {
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			if (obs_node->next_node == NULL) {
				if (new_ending_node != NULL) {
					new_ending_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = new_ending_node->id;
					new_branch_node->original_next_node = new_ending_node;
				} else {
					new_ending_node = new ObsNode();
					new_ending_node->id = wrapper->solution->node_counter;
					wrapper->solution->node_counter++;

					for (map<int, AbstractNode*>::iterator it = wrapper->solution->nodes.begin();
							it != wrapper->solution->nodes.end(); it++) {
						if (it->second->type == NODE_TYPE_OBS) {
							ObsNode* p_obs_node = (ObsNode*)it->second;
							if (p_obs_node->next_node == NULL) {
								p_obs_node->next_node_id = new_ending_node->id;
								p_obs_node->next_node = new_ending_node;

								new_ending_node->ancestor_ids.push_back(p_obs_node->id);

								break;
							}
						}
					}

					wrapper->solution->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					new_ending_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = new_ending_node->id;
					new_branch_node->original_next_node = new_ending_node;
				}
			} else {
				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				obs_node->next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = obs_node->next_node_id;
				new_branch_node->original_next_node = obs_node->next_node;
			}
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
		exit_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = exit_node_id;
		new_branch_node->branch_next_node = exit_node;
	} else {
		new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = new_nodes[0]->id;
		new_branch_node->branch_next_node = new_nodes[0];
	}

	switch (this->node_context->type) {
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			obs_node->next_node_id = new_branch_node->id;
			obs_node->next_node = new_branch_node;
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

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = new_nodes[n_index+1]->id;
			next_node = new_nodes[n_index+1];
		}

		new_nodes[n_index]->next_node_id = next_node_id;
		new_nodes[n_index]->next_node = next_node;

		next_node->ancestor_ids.push_back(new_nodes[n_index]->id);
	}

	wrapper->solution->clean();

	wrapper->solution->timestamp++;
}
