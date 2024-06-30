#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "new_info_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::experiment_activate(AbstractNode*& curr_node,
										   Problem* problem,
										   vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   BranchExperimentHistory* history) {
	bool result;

	if (this->is_pass_through) {
		if (this->best_info_scope != NULL) {
			curr_node = this->info_branch_node;
		} else {
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		}

		result = true;
	} else {
		if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
			result = false;
		} else {
			run_helper.branch_node_ancestors.insert(this->branch_node);

			run_helper.num_decisions++;

			vector<double> new_input_vals(this->new_input_scope_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
				int curr_layer = 0;
				AbstractScopeHistory* curr_scope_history = context.back().scope_history;
				while (true) {
					map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
						this->new_input_node_contexts[i_index][curr_layer]);
					if (it == curr_scope_history->node_histories.end()) {
						break;
					} else {
						if (curr_layer == (int)this->new_input_scope_contexts[i_index].size()-1) {
							switch (it->first->type) {
							case NODE_TYPE_ACTION:
								{
									ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
									new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
								}
								break;
							case NODE_TYPE_BRANCH:
								{
									BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
									new_input_vals[i_index] = branch_node_history->score;
								}
								break;
							case NODE_TYPE_INFO_BRANCH:
								{
									InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
									if (info_branch_node_history->is_branch) {
										new_input_vals[i_index] = 1.0;
									} else {
										new_input_vals[i_index] = -1.0;
									}
								}
								break;
							}
							break;
						} else {
							curr_layer++;
							curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
						}
					}
				}
			}
			this->new_network->activate(new_input_vals);
			double new_predicted_score = this->new_network->output->acti_vals[0];

			#if defined(MDEBUG) && MDEBUG
			bool decision_is_branch;
			if (run_helper.curr_run_seed%2 == 0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
			#else
			bool decision_is_branch = new_predicted_score >= 0.0;
			#endif /* MDEBUG */

			BranchNodeHistory* branch_node_history = new BranchNodeHistory();
			branch_node_history->score = new_predicted_score;
			branch_node_history->index = context.back().scope_history->node_histories.size();
			context.back().scope_history->node_histories[this->branch_node] = branch_node_history;

			if (decision_is_branch) {
				if (this->best_info_scope != NULL) {
					curr_node = this->info_branch_node;
				} else {
					if (this->best_step_types.size() == 0) {
						curr_node = this->best_exit_next_node;
					} else {
						if (this->best_step_types[0] == STEP_TYPE_ACTION) {
							curr_node = this->best_actions[0];
						} else {
							curr_node = this->best_scopes[0];
						}
					}
				}

				result = true;
			} else {
				result = false;
			}
		}
	}

	if (this->parent_experiment == NULL
			|| this->root_experiment->root_state == ROOT_EXPERIMENT_STATE_EXPERIMENT) {
		if (run_helper.experiment_histories.back() == history) {
			history->instance_count++;

			bool is_target = false;
			if (!history->has_target) {
				double target_probability;
				if (history->instance_count > this->average_instances_per_run) {
					target_probability = 0.5;
				} else {
					target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - history->instance_count));
				}
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < target_probability) {
					is_target = true;
				}
			}

			if (is_target) {
				history->has_target = true;

				ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;
				scope_history->callback_experiment_history = history;

				history->experiment_index = context.back().scope_history->node_histories.size();
			}
		}
	}

	return result;
}

void BranchExperiment::experiment_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (!run_helper.exceeded_limit
			&& run_helper.experiment_histories.back()->experiment == this) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();
		if (history->experiments_seen_order.size() == 0) {
			vector<AbstractNode*> possible_node_contexts;
			vector<bool> possible_is_branch;

			for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.begin();
					it != context.back().scope_history->node_histories.end(); it++) {
				if (it->second->index >= history->experiment_index) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->first;

							if (action_node->action.move == ACTION_NOOP) {
								map<int, AbstractNode*>::iterator it = action_node->parent->nodes.find(action_node->id);
								if (it == action_node->parent->nodes.end()) {
									/**
									 * - new ending node edge case
									 */
									continue;
								}
							}

							possible_node_contexts.push_back(it->first);
							possible_is_branch.push_back(false);
						}

						break;
					case NODE_TYPE_SCOPE:
						{
							possible_node_contexts.push_back(it->first);
							possible_is_branch.push_back(false);
						}

						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

							possible_node_contexts.push_back(it->first);
							possible_is_branch.push_back(branch_node_history->score >= 0.0);
						}

						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;

							possible_node_contexts.push_back(it->first);
							possible_is_branch.push_back(info_branch_node_history->is_branch);
						}

						break;
					}
				}
			}

			/**
			 * - possible to be empty due to ending node edge case
			 */
			if (possible_node_contexts.size() > 0) {
				uniform_int_distribution<int> possible_distribution(0, (int)possible_node_contexts.size()-1);
				int rand_index = possible_distribution(generator);

				uniform_int_distribution<int> branch_distribution(0, 3);
				if (branch_distribution(generator) == 0) {
					uniform_int_distribution<int> info_distribution(0, 1);
					if (info_distribution(generator) == 0) {
						NewInfoExperiment* new_experiment = new NewInfoExperiment(
							this->scope_context,
							possible_node_contexts[rand_index],
							possible_is_branch[rand_index],
							this->score_type,
							this);

						/**
						 * - insert at front to match finalize order
						 */
						possible_node_contexts[rand_index]->experiments.insert(possible_node_contexts[rand_index]->experiments.begin(), new_experiment);
					} else {
						BranchExperiment* new_experiment = new BranchExperiment(
							this->scope_context,
							possible_node_contexts[rand_index],
							possible_is_branch[rand_index],
							this->score_type,
							this);

						possible_node_contexts[rand_index]->experiments.insert(possible_node_contexts[rand_index]->experiments.begin(), new_experiment);
					}
				} else {
					PassThroughExperiment* new_experiment = new PassThroughExperiment(
						this->scope_context,
						possible_node_contexts[rand_index],
						possible_is_branch[rand_index],
						this->score_type,
						this);

					possible_node_contexts[rand_index]->experiments.insert(possible_node_contexts[rand_index]->experiments.begin(), new_experiment);
				}
			}
		}
	}
}

void BranchExperiment::experiment_backprop(RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
