#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::experiment_verify_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->is_pass_through) {
		if (this->throw_id != -1) {
			run_helper.throw_id = -1;
		}

		if (this->best_step_types.size() == 0) {
			if (this->exit_node != NULL) {
				curr_node = this->exit_node;
			} else {
				curr_node = this->best_exit_next_node;
			}
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}
	} else {
		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				action_node->hook_obs_indexes.push_back(this->input_obs_indexes[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			}
		}
		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		input_vals_helper(0,
						  this->input_max_depth,
						  scope_context,
						  node_context,
						  input_vals,
						  context[context.size() - this->scope_context.size()].scope_history);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
				action_node->hook_obs_indexes.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}

		double existing_predicted_score = this->existing_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
		}
		if (this->existing_network != NULL) {
			vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
				existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
					existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
				}
			}
			this->existing_network->activate(existing_network_input_vals);
			existing_predicted_score += this->existing_network->output->acti_vals[0];
		}

		double new_predicted_score = this->new_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
		}
		if (this->new_network != NULL) {
			vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
				new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
					new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
				}
			}
			this->new_network->activate(new_network_input_vals);
			new_predicted_score += this->new_network->output->acti_vals[0];
		}

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
		#endif /* MDEBUG */

		if (decision_is_branch) {
			if (this->throw_id != -1) {
				run_helper.throw_id = -1;
			}

			if (this->best_step_types.size() == 0) {
				if (this->exit_node != NULL) {
					curr_node = this->exit_node;
				} else {
					curr_node = this->best_exit_next_node;
				}
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		}
	}
}

void BranchExperiment::experiment_verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->root_state == ROOT_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
		this->combined_score /= VERIFY_1ST_NUM_DATAPOINTS;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->combined_score > this->verify_existing_average_score) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

			this->root_state = ROOT_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			/**
			 * - leave this->experiment_iter unchanged
			 */
		} else {
			bool to_experiment = false;
			if (this->verify_experiments.back()->type == EXPERIMENT_TYPE_BRANCH) {
				double combined_branch_weight = 1.0;
				AbstractExperiment* curr_experiment = this->verify_experiments.back();
				while (true) {
					if (curr_experiment == NULL) {
						break;
					}

					if (curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
						combined_branch_weight *= branch_experiment->branch_weight;
					}
					curr_experiment = curr_experiment->parent_experiment;
				}

				BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
				if (branch_experiment->best_step_types.size() > 0
						&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
					#if defined(MDEBUG) && MDEBUG
					for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
						delete branch_experiment->verify_problems[p_index];
					}
					branch_experiment->verify_problems.clear();
					branch_experiment->verify_seeds.clear();
					branch_experiment->verify_original_scores.clear();
					branch_experiment->verify_branch_scores.clear();
					/**
					 * - simply rely on leaf experiment to verify
					 */
					#endif /* MDEBUG */

					branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			} else {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
				if (pass_through_experiment->best_step_types.size() > 0) {
					pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			}

			if (!to_experiment) {
				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

				curr_experiment->experiment_iter++;
				int matching_index;
				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
						matching_index = c_index;
						break;
					}
				}
				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
				Solution* empty = NULL;
				this->verify_experiments.back()->finalize(empty);
				delete this->verify_experiments.back();

				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
					* pow(0.5, this->verify_experiments.size());
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						break;
					}

					if (curr_experiment->experiment_iter >= target_count) {
						AbstractExperiment* parent = curr_experiment->parent_experiment;

						parent->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
							if (parent->child_experiments[c_index] == curr_experiment) {
								matching_index = c_index;
								break;
							}
						}
						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
						Solution* empty = NULL;
						curr_experiment->finalize(empty);
						delete curr_experiment;

						curr_experiment = parent;
						target_count *= 2.0;
					} else {
						break;
					}
				}
			}

			this->verify_experiments.clear();

			if (this->experiment_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			}
		}
	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
		this->combined_score /= VERIFY_2ND_NUM_DATAPOINTS;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->combined_score > this->verify_existing_average_score) {
		#endif /* MDEBUG */
			cout << "experiment success" << endl;
			cout << "this->scope_context:" << endl;
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				cout << c_index << ": " << this->scope_context[c_index]->id << endl;
			}
			cout << "this->node_context:" << endl;
			for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				cout << c_index << ": " << this->node_context[c_index]->id << endl;
			}
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "this->throw_id: " << this->throw_id << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " E";
				}
			}
			cout << endl;

			cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}
			cout << "this->best_exit_throw_id: " << this->best_exit_throw_id << endl;

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;

			/**
			 * - also finalize this->verify_experiments in finalize()
			 */

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			bool to_experiment = false;
			if (this->verify_experiments.back()->type == EXPERIMENT_TYPE_BRANCH) {
				double combined_branch_weight = 1.0;
				AbstractExperiment* curr_experiment = this->verify_experiments.back();
				while (true) {
					if (curr_experiment == NULL) {
						break;
					}

					if (curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
						combined_branch_weight *= branch_experiment->branch_weight;
					}
					curr_experiment = curr_experiment->parent_experiment;
				}

				BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
				if (branch_experiment->best_step_types.size() > 0
						&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
					#if defined(MDEBUG) && MDEBUG
					for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
						delete branch_experiment->verify_problems[p_index];
					}
					branch_experiment->verify_problems.clear();
					branch_experiment->verify_seeds.clear();
					branch_experiment->verify_original_scores.clear();
					branch_experiment->verify_branch_scores.clear();
					/**
					 * - simply rely on leaf experiment to verify
					 */
					#endif /* MDEBUG */

					branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			} else {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
				if (pass_through_experiment->best_step_types.size() > 0) {
					pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			}

			if (!to_experiment) {
				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

				curr_experiment->experiment_iter++;
				int matching_index;
				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
						matching_index = c_index;
						break;
					}
				}
				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
				Solution* empty = NULL;
				this->verify_experiments.back()->finalize(empty);
				delete this->verify_experiments.back();

				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
					* pow(0.5, this->verify_experiments.size());
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						break;
					}

					if (curr_experiment->experiment_iter >= target_count) {
						AbstractExperiment* parent = curr_experiment->parent_experiment;

						parent->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
							if (parent->child_experiments[c_index] == curr_experiment) {
								matching_index = c_index;
								break;
							}
						}
						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
						Solution* empty = NULL;
						curr_experiment->finalize(empty);
						delete curr_experiment;

						curr_experiment = parent;
						target_count *= 2.0;
					} else {
						break;
					}
				}
			}

			this->verify_experiments.clear();

			if (this->experiment_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			}
		}
	}
}
