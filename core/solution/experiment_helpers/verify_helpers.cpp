#include "experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void Experiment::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper) {
	if (this->is_pass_through) {
		if (this->throw_id != -1) {
			run_helper.throw_id = -1;
		}

		if (this->step_types.size() == 0) {
			if (this->exit_node != NULL) {
				curr_node = this->exit_node;
			} else {
				curr_node = this->exit_next_node;
			}
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
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

			if (this->step_types.size() == 0) {
				if (this->exit_node != NULL) {
					curr_node = this->exit_node;
				} else {
					curr_node = this->exit_next_node;
				}
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}
}

void Experiment::verify_backprop(double target_val,
								 RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state == EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double combined_improvement = this->combined_score - this->verify_existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (this->verify_existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		double combined_branch_weight = 1.0;
		Experiment* curr_experiment = this;
		while (true) {
			if (curr_experiment == NULL) {
				break;
			}

			combined_branch_weight *= curr_experiment->branch_weight;
			curr_experiment = curr_experiment->parent_experiment;
		}

		if (this->new_is_better
				&& this->branch_weight > 0.01
				&& combined_improvement_t_score > 1.960) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		#if defined(MDEBUG) && MDEBUG
		} else if (this->step_types.size() > 0
				&& rand()%4 == 0) {
		#else
		} else if (this->step_types.size() > 0
				&& this->combined_score >= this->verify_existing_average_score
				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
		#endif /* MDEBUG */
			this->state = EXPERIMENT_STATE_EXPERIMENT;
			this->experiment_iter = 0;
		} else {
			this->explore_iter++;
			if (this->explore_iter < MAX_EXPLORE_TRIES) {
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->actions[s_index];
					} else {
						delete this->scopes[s_index];
					}
				}

				this->step_types.clear();
				this->actions.clear();
				this->scopes.clear();
				this->catch_throw_ids.clear();

				if (this->branch_node != NULL) {
					delete this->branch_node;
					this->branch_node = NULL;
				}
				if (this->exit_node != NULL) {
					delete this->exit_node;
					this->exit_node = NULL;
				}

				this->new_linear_weights.clear();
				this->new_network_input_indexes.clear();
				if (this->new_network != NULL) {
					delete this->new_network;
					this->new_network = NULL;
				}

				uniform_int_distribution<int> explore_distribution(0, 9);
				if (explore_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_NEUTRAL;
				} else {
					this->explore_type = EXPLORE_TYPE_GOOD;
				}

				this->state = EXPERIMENT_STATE_EXPLORE_CREATE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double combined_improvement = this->combined_score - this->verify_existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (this->verify_existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		double combined_branch_weight = 1.0;
		Experiment* curr_experiment = this;
		while (true) {
			if (curr_experiment == NULL) {
				break;
			}

			combined_branch_weight *= curr_experiment->branch_weight;
			curr_experiment = curr_experiment->parent_experiment;
		}

		cout << "combined_branch_weight: " << combined_branch_weight << endl;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->new_is_better
				&& this->branch_weight > 0.01
				&& combined_improvement_t_score > 1.960) {
		#endif /* MDEBUG */
			cout << "verify" << endl;
			cout << "this->parent_experiment: " << this->parent_experiment << endl;
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
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[s_index]->action.move;
				} else {
					cout << " E";
				}
			}
			cout << endl;

			cout << "this->exit_depth: " << this->exit_depth << endl;
			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}
			cout << "this->exit_throw_id: " << this->exit_throw_id << endl;

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;
			cout << "this->verify_existing_score_standard_deviation: " << this->verify_existing_score_standard_deviation << endl;
			cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

			cout << "this->branch_weight: " << this->branch_weight << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->is_pass_through) {
				if (this->parent_experiment == NULL) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					vector<Experiment*> verify_experiments;
					verify_experiments.insert(verify_experiments.begin(), this);
					Experiment* curr_experiment = this->parent_experiment;
					while (true) {
						if (curr_experiment->parent_experiment == NULL) {
							/**
							 * - don't include root
							 */
							break;
						} else {
							verify_experiments.insert(verify_experiments.begin(), curr_experiment);
							curr_experiment = curr_experiment->parent_experiment;
						}
					}

					this->root_experiment->verify_experiments = verify_experiments;

					this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

					this->root_experiment->state = EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING;

					this->state = EXPERIMENT_STATE_ROOT_VERIFY;
				}
			} else {
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
			}
			#else
			if (this->parent_experiment == NULL) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				vector<Experiment*> verify_experiments;
				verify_experiments.insert(verify_experiments.begin(), this);
				Experiment* curr_experiment = this->parent_experiment;
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						/**
						 * - don't include root
						 */
						break;
					} else {
						verify_experiments.insert(verify_experiments.begin(), curr_experiment);
						curr_experiment = curr_experiment->parent_experiment;
					}
				}

				this->root_experiment->verify_experiments = verify_experiments;

				this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

				this->root_experiment->state = EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING;

				this->state = EXPERIMENT_STATE_ROOT_VERIFY;
			}
			#endif /* MDEBUG */
		#if defined(MDEBUG) && MDEBUG
		} else if (this->step_types.size() > 0
				&& rand()%4 == 0) {
		#else
		} else if (this->step_types.size() > 0
				&& this->combined_score >= this->verify_existing_average_score
				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
		#endif /* MDEBUG */
			this->state = EXPERIMENT_STATE_EXPERIMENT;
			this->experiment_iter = 0;
		} else {
			this->explore_iter++;
			if (this->explore_iter < MAX_EXPLORE_TRIES) {
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->actions[s_index];
					} else {
						delete this->scopes[s_index];
					}
				}

				this->step_types.clear();
				this->actions.clear();
				this->scopes.clear();
				this->catch_throw_ids.clear();

				if (this->branch_node != NULL) {
					delete this->branch_node;
					this->branch_node = NULL;
				}
				if (this->exit_node != NULL) {
					delete this->exit_node;
					this->exit_node = NULL;
				}

				this->new_linear_weights.clear();
				this->new_network_input_indexes.clear();
				if (this->new_network != NULL) {
					delete this->new_network;
					this->new_network = NULL;
				}

				uniform_int_distribution<int> explore_distribution(0, 9);
				if (explore_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_NEUTRAL;
				} else {
					this->explore_type = EXPLORE_TYPE_GOOD;
				}

				this->state = EXPERIMENT_STATE_EXPLORE_CREATE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
