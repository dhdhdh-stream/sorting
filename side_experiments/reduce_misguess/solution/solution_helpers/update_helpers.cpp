/**
 * - OK to train on only paths taken:
 *   - initially, paths not taken are ones that have low score
 *   - since bad paths are no longer taken, score increase overall...
 *   - ...increasing score for bad paths, leading to them being taken again
 *   - ultimately, bad paths will be predicted as bad, but not accurately
 * 
 * - do not batch train
 *   - adam easily overfits(?)
 */

#include "solution_helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MIN_BATCH_SIZE = 2;
const int BATCH_SIZE = 10;
const int ITERS_PER_BATCH = 2;
#else
const int MIN_BATCH_SIZE = 100;
const int BATCH_SIZE = 1000;
const int ITERS_PER_BATCH = 20;
#endif /* MDEBUG */

void update_helper(ScopeHistory* scope_history,
				   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)node;

				noop_node->curr_num_instances++;

				for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
					noop_node->experiments[e_index]->backprop(h_it->second->obs,
															  target_val);
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;

				action_node->curr_num_instances++;

				for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
					action_node->experiments[e_index]->backprop(h_it->second->obs,
																target_val);
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				update_helper(scope_node_history->scope_history,
							  target_val);

				scope_node->curr_num_instances++;

				for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
					scope_node->experiments[e_index]->backprop(h_it->second->obs,
															   target_val);
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;

				if (branch_node_history->is_branch) {
					branch_node->branch_obs_histories.back().push_back(branch_node_history->obs);

					branch_node->branch_curr_num_instances++;

					branch_node->consec_original = 0.0;
					branch_node->consec_branch += 1.0 / branch_node->branch_average_instances_per_hit;

					for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
						branch_node->branch_experiments[e_index]->backprop(h_it->second->obs,
																		   target_val);
					}
				} else {
					branch_node->original_obs_histories.back().push_back(branch_node_history->obs);

					branch_node->original_curr_num_instances++;

					branch_node->consec_original += 1.0 / branch_node->original_average_instances_per_hit;
					branch_node->consec_branch = 0.0;

					for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
						branch_node->original_experiments[e_index]->backprop(h_it->second->obs,
																			 target_val);
					}
				}
			}
			break;
		}
	}
}

void update_helper(SolutionWrapper* wrapper,
				   double target_val) {
	wrapper->solution->curr_score = 0.999*wrapper->solution->curr_score + 0.001*target_val;

	update_helper(wrapper->scope_histories[0],
				  target_val);

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)it->second;
					noop_node->average_instances_per_run = 0.999*noop_node->average_instances_per_run + 0.001*noop_node->curr_num_instances;
					if (noop_node->curr_num_instances > 0) {
						noop_node->average_instances_per_hit = 0.999*noop_node->average_instances_per_hit + 0.001*noop_node->curr_num_instances;
						noop_node->val_average = 0.999*noop_node->val_average + 0.001*target_val;

						for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
							noop_node->experiments[e_index]->update();
						}

						noop_node->curr_num_instances = 0;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;
					action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run + 0.001*action_node->curr_num_instances;
					if (action_node->curr_num_instances > 0) {
						action_node->average_instances_per_hit = 0.999*action_node->average_instances_per_hit + 0.001*action_node->curr_num_instances;
						action_node->val_average = 0.999*action_node->val_average + 0.001*target_val;

						for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
							action_node->experiments[e_index]->update();
						}

						action_node->curr_num_instances = 0;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					scope_node->average_instances_per_run = 0.999*scope_node->average_instances_per_run + 0.001*scope_node->curr_num_instances;
					if (scope_node->curr_num_instances > 0) {
						scope_node->average_instances_per_hit = 0.999*scope_node->average_instances_per_hit + 0.001*scope_node->curr_num_instances;
						scope_node->val_average = 0.999*scope_node->val_average + 0.001*target_val;

						for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
							scope_node->experiments[e_index]->update();
						}

						scope_node->curr_num_instances = 0;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->original_average_instances_per_run = 0.999*branch_node->original_average_instances_per_run + 0.001*branch_node->original_curr_num_instances;
					if (branch_node->original_curr_num_instances > 0) {
						branch_node->original_average_instances_per_hit = 0.999*branch_node->original_average_instances_per_hit + 0.001*branch_node->original_curr_num_instances;
						branch_node->original_val_average = 0.999*branch_node->original_val_average + 0.001*target_val;

						branch_node->original_target_val_histories.push_back(target_val);
						if (branch_node->original_obs_histories.size() >= MIN_BATCH_SIZE) {
							uniform_int_distribution<int> sample_distribution(0, branch_node->original_obs_histories.size()-1);
							for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
								int sample_index = sample_distribution(generator);
								for (int i_index = 0; i_index < (int)branch_node->original_obs_histories[sample_index].size(); i_index++) {
									branch_node->original_score_network->activate(branch_node->original_obs_histories[sample_index][i_index]);
									branch_node->original_misguess_network->activate(branch_node->original_obs_histories[sample_index][i_index]);

									double score_error = branch_node->original_target_val_histories[sample_index] - branch_node->original_score_network->output->acti_vals(0);

									double misguess = abs(score_error);
									double misguess_error = misguess - branch_node->original_misguess_network->output->acti_vals(0);

									branch_node->original_score_network->backprop(score_error);
									branch_node->original_misguess_network->backprop(misguess_error);
								}

								branch_node->original_score_network->epoch_iter++;
								if (branch_node->original_score_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
									branch_node->original_score_network->update();
									branch_node->original_misguess_network->update();

									branch_node->original_score_network->epoch_iter = 0;
								}
							}
						}
						branch_node->original_obs_histories.push_back(vector<vector<double>>());
						if (branch_node->original_obs_histories.size() > BATCH_SIZE) {
							branch_node->original_obs_histories.erase(branch_node->original_obs_histories.begin());
							branch_node->original_target_val_histories.erase(branch_node->original_target_val_histories.begin());
						}

						for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
							branch_node->original_experiments[e_index]->update();
						}

						branch_node->original_curr_num_instances = 0;
					}
					branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run + 0.001*branch_node->branch_curr_num_instances;
					if (branch_node->branch_curr_num_instances > 0) {
						branch_node->branch_average_instances_per_hit = 0.999*branch_node->branch_average_instances_per_hit + 0.001*branch_node->branch_curr_num_instances;
						branch_node->branch_val_average = 0.999*branch_node->branch_val_average + 0.001*target_val;

						branch_node->branch_target_val_histories.push_back(target_val);
						if (branch_node->branch_obs_histories.size() >= MIN_BATCH_SIZE) {
							uniform_int_distribution<int> sample_distribution(0, branch_node->branch_obs_histories.size()-1);
							for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
								int sample_index = sample_distribution(generator);
								for (int i_index = 0; i_index < (int)branch_node->branch_obs_histories[sample_index].size(); i_index++) {
									branch_node->branch_score_network->activate(branch_node->branch_obs_histories[sample_index][i_index]);
									branch_node->branch_misguess_network->activate(branch_node->branch_obs_histories[sample_index][i_index]);

									double score_error = branch_node->branch_target_val_histories[sample_index] - branch_node->branch_score_network->output->acti_vals(0);

									double misguess = abs(score_error);
									double misguess_error = misguess - branch_node->branch_misguess_network->output->acti_vals(0);

									branch_node->branch_score_network->backprop(score_error);
									branch_node->branch_misguess_network->backprop(misguess_error);
								}

								branch_node->branch_score_network->epoch_iter++;
								if (branch_node->branch_score_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
									branch_node->branch_score_network->update();
									branch_node->branch_misguess_network->update();

									branch_node->branch_score_network->epoch_iter = 0;
								}
							}
						}
						branch_node->branch_obs_histories.push_back(vector<vector<double>>());
						if (branch_node->branch_obs_histories.size() > BATCH_SIZE) {
							branch_node->branch_obs_histories.erase(branch_node->branch_obs_histories.begin());
							branch_node->branch_target_val_histories.erase(branch_node->branch_target_val_histories.begin());
						}

						for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
							branch_node->branch_experiments[e_index]->update();
						}

						branch_node->branch_curr_num_instances = 0;
					}
				}
				break;
			}
		}
	}

	for (int s_index = 0; s_index < (int)wrapper->ramp_tracker.size(); s_index++) {
		for (map<BranchNode*, bool>::iterator it = wrapper->ramp_tracker[s_index].begin();
				it != wrapper->ramp_tracker[s_index].end(); it++) {
			if (it->second) {
				it->first->new_sum_score += target_val;
				it->first->new_count++;
			} else {
				it->first->existing_sum_score += target_val;
				it->first->existing_count++;
			}

			if (it->first->new_count + it->first->existing_count >= ITERS_PER_RAMP) {
				double existing_score = it->first->existing_sum_score / it->first->existing_count;
				double new_score = it->first->new_sum_score / it->first->new_count;
				if (new_score > existing_score) {
					it->first->ramp++;
				} else {
					it->first->ramp--;
				}

				it->first->existing_sum_score = 0.0;
				it->first->existing_count = 0;
				it->first->new_sum_score = 0.0;
				it->first->new_count = 0;
			}
		}
	}
}
