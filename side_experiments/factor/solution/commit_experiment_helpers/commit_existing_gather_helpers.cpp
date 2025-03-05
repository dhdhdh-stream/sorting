#include "commit_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void CommitExperiment::commit_existing_gather_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	run_helper.has_explore = true;

	for (int n_index = 0; n_index < this->step_iter; n_index++) {
		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		}
	}

	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		if (this->commit_existing_inputs.size() < NETWORK_NUM_INPUTS) {
			vector<Scope*> scope_context;
			vector<int> node_context;
			int node_count = 0;
			pair<pair<vector<Scope*>,vector<int>>,pair<int,int>> new_input;
			gather_possible_helper(scope_history,
								   scope_context,
								   node_context,
								   node_count,
								   new_input);

			bool is_existing = false;
			for (int i_index = 0; i_index < (int)this->commit_existing_inputs.size(); i_index++) {
				if (new_input == this->commit_existing_inputs[i_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->commit_existing_inputs.push_back(new_input);
			}
		}

		for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
			if (this->commit_existing_factor_ids.size() < GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
				pair<int,int> new_factor = {-1, -1};
				gather_possible_factor_helper(scope_history,
											  new_factor);

				if (new_factor.first != -1) {
					bool is_existing = false;
					for (int i_index = 0; i_index < (int)this->commit_existing_factor_ids.size(); i_index++) {
						if (new_factor == this->commit_existing_factor_ids[i_index]) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						this->commit_existing_factor_ids.push_back(new_factor);
					}
				}
			}
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);
	}

	for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
		if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->save_actions[s_index]);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
			this->save_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		run_helper.num_actions += 2;
	}

	curr_node = this->save_exit_next_node;
}

void CommitExperiment::commit_existing_gather_backprop() {
	this->state_iter++;
	if (this->state_iter >= GATHER_ITERS) {
		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING;
		this->state_iter = 0;
	}
}
