#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "new_info_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::verify_activate(AbstractNode*& curr_node,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper) {
	if (this->is_pass_through) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		run_helper.num_decisions++;

		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								if (branch_node_history->is_branch) {
									input_vals[i_index] = 1.0;
								} else {
									input_vals[i_index] = -1.0;
								}
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									input_vals[i_index] = 1.0;
								} else {
									input_vals[i_index] = -1.0;
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
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}

			return true;
		} else {
			return false;
		}
	}
}

void BranchExperiment::verify_backprop(double target_val,
									   RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state == BRANCH_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
		this->combined_score /= VERIFY_1ST_NUM_DATAPOINTS;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double combined_branch_weight = 1.0;
		AbstractExperiment* curr_experiment = this;
		while (true) {
			if (curr_experiment == NULL) {
				break;
			}

			switch (curr_experiment->type) {
			case EXPERIMENT_TYPE_BRANCH:
				{
					BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
					combined_branch_weight *= branch_experiment->branch_weight;
				}
				break;
			case EXPERIMENT_TYPE_NEW_INFO:
				{
					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
					combined_branch_weight *= new_info_experiment->branch_weight;
				}
				break;
			}
			curr_experiment = curr_experiment->parent_experiment;
		}

		if (this->combined_score > this->verify_existing_average_score) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

			this->state = BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			this->explore_iter++;
			if (this->explore_iter < MAX_EXPLORE_TRIES) {
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_step_types.clear();
				this->best_actions.clear();
				this->best_scopes.clear();

				if (this->ending_node != NULL) {
					delete this->ending_node;
					this->ending_node = NULL;
				}

				this->new_linear_weights.clear();
				this->new_network_input_indexes.clear();
				if (this->new_network != NULL) {
					delete this->new_network;
					this->new_network = NULL;
				}

				uniform_int_distribution<int> neutral_distribution(0, 9);
				if (neutral_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_NEUTRAL;
				} else {
					uniform_int_distribution<int> best_distribution(0, 1);
					if (best_distribution(generator) == 0) {
						this->explore_type = EXPLORE_TYPE_BEST;

						this->best_surprise = 0.0;
					} else {
						this->explore_type = EXPLORE_TYPE_GOOD;
					}
				}

				this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
		this->combined_score /= VERIFY_2ND_NUM_DATAPOINTS;

		double combined_branch_weight = 1.0;
		AbstractExperiment* curr_experiment = this;
		while (true) {
			if (curr_experiment == NULL) {
				break;
			}

			switch (curr_experiment->type) {
			case EXPERIMENT_TYPE_BRANCH:
				{
					BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
					combined_branch_weight *= branch_experiment->branch_weight;
				}
				break;
			case EXPERIMENT_TYPE_NEW_INFO:
				{
					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
					combined_branch_weight *= new_info_experiment->branch_weight;
				}
				break;
			}
			curr_experiment = curr_experiment->parent_experiment;
		}

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->combined_score > this->verify_existing_average_score) {
		#endif /* MDEBUG */
			cout << "verify" << endl;
			cout << "this->parent_experiment: " << this->parent_experiment << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " E";
				}
			}
			cout << endl;

			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;

			cout << "this->branch_weight: " << this->branch_weight << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->is_pass_through) {
				if (this->parent_experiment == NULL) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					vector<AbstractExperiment*> verify_experiments;
					verify_experiments.insert(verify_experiments.begin(), this);
					AbstractExperiment* curr_experiment = this->parent_experiment;
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

					this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

					this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;

					this->state = BRANCH_EXPERIMENT_STATE_ROOT_VERIFY;
				}
			} else {
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
			}
			#else
			if (this->parent_experiment == NULL) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				vector<AbstractExperiment*> verify_experiments;
				verify_experiments.insert(verify_experiments.begin(), this);
				AbstractExperiment* curr_experiment = this->parent_experiment;
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

				this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

				this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;

				this->state = BRANCH_EXPERIMENT_STATE_ROOT_VERIFY;
			}
			#endif /* MDEBUG */
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0
				&& rand()%4 == 0) {
		#else
		} else if (this->best_step_types.size() > 0
				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
		#endif /* MDEBUG */
			this->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
			this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			this->experiment_iter = 0;
		} else {
			this->explore_iter++;
			if (this->explore_iter < MAX_EXPLORE_TRIES) {
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_step_types.clear();
				this->best_actions.clear();
				this->best_scopes.clear();

				if (this->ending_node != NULL) {
					delete this->ending_node;
					this->ending_node = NULL;
				}

				this->new_linear_weights.clear();
				this->new_network_input_indexes.clear();
				if (this->new_network != NULL) {
					delete this->new_network;
					this->new_network = NULL;
				}

				uniform_int_distribution<int> neutral_distribution(0, 9);
				if (neutral_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_NEUTRAL;
				} else {
					uniform_int_distribution<int> best_distribution(0, 1);
					if (best_distribution(generator) == 0) {
						this->explore_type = EXPLORE_TYPE_BEST;

						this->best_surprise = 0.0;
					} else {
						this->explore_type = EXPLORE_TYPE_GOOD;
					}
				}

				this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
