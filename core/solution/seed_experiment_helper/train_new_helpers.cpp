#include "seed_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_set.h"

using namespace std;

void SeedExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		SeedExperimentHistory* history) {
	this->num_instances_until_target--;

	if (this->num_instances_until_target == 0) {
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			history->predicted_scores.push_back(vector<double>());
			break;
		case SCORE_TYPE_ALL:
			history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
			for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
				ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
				Scope* scope = (Scope*)scope_history->scope;

				if (scope->eval_network != NULL) {
					scope_history->callback_experiment_history = history;
					scope_history->callback_experiment_indexes.push_back(
						(int)history->predicted_scores.size()-1);
					scope_history->callback_experiment_layers.push_back(l_index);
				}
			}
			break;
		}

		for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
			if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_seed_actions[s_index]->explore_activate(
					problem,
					context.back().scope_history->node_histories);
			} else {
				this->best_seed_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper,
					context.back().scope_history->node_histories);
			}
		}

		this->scope_histories.push_back(context.back().scope_history->deep_copy());

		curr_node = this->best_seed_exit_next_node;

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void SeedExperiment::train_new_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void SeedExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double final_score;
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			if (run_helper.exceeded_limit) {
				final_score = -10.0;
			} else {
				final_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
			}
			break;
		case SCORE_TYPE_ALL:
			{
				double sum_score;
				if (run_helper.exceeded_limit) {
					sum_score = -10.0;
				} else {
					sum_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
				}
				for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
					sum_score += history->predicted_scores[i_index][l_index];
				}
				final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
			}
			break;
		}

		double surprise = final_score - this->existing_average_score;

		this->target_val_histories.push_back(surprise);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		default_random_engine generator_copy = generator;
		shuffle(this->scope_histories.begin(), this->scope_histories.end(), generator);
		shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);

		int num_instances = (int)this->target_val_histories.size();

		vector<vector<double>> inputs(num_instances);
		#if defined(MDEBUG) && MDEBUG
		#else
		double average_misguess;
		double misguess_standard_deviation;
		#endif /* MDEBUG */

		int train_index = 0;
		while (true) {
			vector<vector<AbstractScope*>> possible_scope_contexts;
			vector<vector<AbstractNode*>> possible_node_contexts;
			vector<int> possible_obs_indexes;

			vector<AbstractScope*> scope_context;
			vector<AbstractNode*> node_context;
			uniform_int_distribution<int> history_distribution(0, num_instances-1);
			gather_possible_helper(scope_context,
								   node_context,
								   possible_scope_contexts,
								   possible_node_contexts,
								   possible_obs_indexes,
								   this->scope_histories[history_distribution(generator)]);

			if (possible_node_contexts.size() > 0) {
				vector<vector<AbstractScope*>> test_input_scope_contexts = this->new_input_scope_contexts;
				vector<vector<AbstractNode*>> test_input_node_contexts = this->new_input_node_contexts;
				vector<int> test_input_obs_indexes = this->new_input_obs_indexes;

				vector<int> remaining_indexes(possible_scope_contexts.size());
				for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
					remaining_indexes[p_index] = p_index;
				}
				int num_new_input = 0;
				while (true) {
					uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
					int rand_index = distribution(generator);

					bool can_add = true;
					for (int s_index = train_index+1; s_index < (int)this->best_seed_step_types.size(); s_index++) {
						if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
							if (possible_node_contexts[remaining_indexes[rand_index]][0] == this->best_seed_actions[s_index]) {
								can_add = false;
							}
						} else {
							if (possible_node_contexts[remaining_indexes[rand_index]][0] == this->best_seed_scopes[s_index]) {
								can_add = false;
							}
						}
					}
					for (int i_index = 0; i_index < (int)test_input_node_contexts.size(); i_index++) {
						if (possible_scope_contexts[remaining_indexes[rand_index]] == test_input_scope_contexts[i_index]
								&& possible_node_contexts[remaining_indexes[rand_index]] == test_input_node_contexts[i_index]
								&& possible_obs_indexes[remaining_indexes[rand_index]] == test_input_obs_indexes[i_index]) {
							can_add = false;
							break;
						}
					}
					if (can_add) {
						test_input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
						test_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
						test_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
						num_new_input++;
					}

					remaining_indexes.erase(remaining_indexes.begin() + rand_index);

					if (num_new_input >= NETWORK_INCREMENT_NUM_NEW
							|| remaining_indexes.size() == 0) {
						break;
					}
				}

				Network* test_network;
				if (this->new_network == NULL) {
					test_network = new Network();
				} else {
					test_network = new Network(this->new_network);
				}
				test_network->increment((int)test_input_scope_contexts.size());

				vector<vector<double>> test_inputs = inputs;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					test_inputs[d_index].reserve((int)test_input_scope_contexts.size());
					for (int t_index = (int)this->new_input_scope_contexts.size(); t_index < (int)test_input_scope_contexts.size(); t_index++) {
						int curr_layer = 0;
						AbstractScopeHistory* curr_scope_history = this->scope_histories[d_index];
						while (true) {
							map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
								test_input_node_contexts[t_index][curr_layer]);
							if (it == curr_scope_history->node_histories.end()) {
								test_inputs[d_index].push_back(0.0);
								break;
							} else {
								if (curr_layer == (int)test_input_scope_contexts[t_index].size()-1) {
									switch (it->first->type) {
									case NODE_TYPE_ACTION:
										{
											ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
											test_inputs[d_index].push_back(action_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
										}
										break;
									case NODE_TYPE_BRANCH:
										{
											BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
											test_inputs[d_index].push_back(branch_node_history->score);
										}
										break;
									case NODE_TYPE_INFO_BRANCH:
										{
											InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
											if (info_branch_node_history->is_branch) {
												test_inputs[d_index].push_back(1.0);
											} else {
												test_inputs[d_index].push_back(-1.0);
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
				}

				train_w_drop_network(test_inputs,
									 this->target_val_histories,
									 test_network);

				double test_average_misguess;
				double test_misguess_standard_deviation;
				measure_network(test_inputs,
								this->target_val_histories,
								test_network,
								test_average_misguess,
								test_misguess_standard_deviation);

				bool is_select = false;
				if (this->new_network == NULL) {
					is_select = true;
				} else {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%3 == 0) {
					#else
					double improvement = average_misguess - test_average_misguess;
					double standard_deviation = min(misguess_standard_deviation, test_misguess_standard_deviation);
					double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

					if (t_score > 1.645) {
					#endif /* MDEBUG */
						is_select = true;
					}
				}

				if (is_select) {
					cout << "s" << endl;

					int original_input_size = (int)this->new_input_scope_contexts.size();
					int test_input_size = (int)test_input_scope_contexts.size();

					#if defined(MDEBUG) && MDEBUG
					#else
					average_misguess = test_average_misguess;
					misguess_standard_deviation = test_misguess_standard_deviation;
					#endif /* MDEBUG */

					this->new_input_scope_contexts = test_input_scope_contexts;
					this->new_input_node_contexts = test_input_node_contexts;
					this->new_input_obs_indexes = test_input_obs_indexes;

					if (this->new_network != NULL) {
						delete this->new_network;
					}
					this->new_network = test_network;

					inputs = test_inputs;

					for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
						vector<vector<AbstractScope*>> remove_test_input_scope_contexts = this->new_input_scope_contexts;
						vector<vector<AbstractNode*>> remove_test_input_node_contexts = this->new_input_node_contexts;
						vector<int> remove_test_input_obs_indexes = this->new_input_obs_indexes;

						remove_test_input_scope_contexts.erase(remove_test_input_scope_contexts.begin() + i_index);
						remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
						remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

						Network* remove_test_network = new Network(this->new_network);
						remove_test_network->remove_input(i_index);

						vector<vector<double>> remove_test_inputs = inputs;
						for (int d_index = 0; d_index < num_instances; d_index++) {
							remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
						}

						optimize_w_drop_network(remove_test_inputs,
												this->target_val_histories,
												remove_test_network);

						double remove_test_average_misguess;
						double remove_test_misguess_standard_deviation;
						measure_network(remove_test_inputs,
										this->target_val_histories,
										remove_test_network,
										remove_test_average_misguess,
										remove_test_misguess_standard_deviation);

						#if defined(MDEBUG) && MDEBUG
						if (rand()%2 == 0) {
						#else
						double remove_improvement = average_misguess - remove_test_average_misguess;
						double remove_standard_deviation = min(misguess_standard_deviation, remove_test_misguess_standard_deviation);
						double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

						if (remove_t_score > -0.674) {
						#endif /* MDEBUG */
							this->new_input_scope_contexts = remove_test_input_scope_contexts;
							this->new_input_node_contexts = remove_test_input_node_contexts;
							this->new_input_obs_indexes = remove_test_input_obs_indexes;

							delete this->new_network;
							this->new_network = remove_test_network;

							inputs = remove_test_inputs;
						} else {
							delete remove_test_network;
						}
					}

					#if defined(MDEBUG) && MDEBUG
					#else
					train_index = 0;
					#endif /* MDEBUG */
				} else {
					delete test_network;

					train_index++;
				}
			} else {
				train_index++;
			}

			cout << "i" << endl;

			int max_iters = max(3, (int)this->best_seed_step_types.size());
			if (train_index >= max_iters) {
				break;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		int positive_count;
		int negative_count;
		uniform_int_distribution<int> distribution(0, 7);
		switch (distribution(generator)) {
		case 0:
			positive_count = 0;
			negative_count = 1;
			break;
		case 1:
			positive_count = 1;
			negative_count = 0;
			break;
		default:
			positive_count = 1;
			negative_count = 1;
			break;
		}
		#else
		int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();
		int test_instances = inputs.size() - train_instances;
		int positive_count = 0;
		int negative_count = 0;
		for (int d_index = 0; d_index < test_instances; d_index++) {
			this->new_network->activate(inputs[train_instances + d_index]);
			if (this->new_network->output->acti_vals[0] >= 0.0) {
				positive_count++;
			} else {
				negative_count++;
			}
		}
		#endif /* MDEBUG */
		if (positive_count == 0) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else if (negative_count == 0) {
			this->is_pass_through = true;

			this->combined_score = 0.0;

			this->state = SEED_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			this->is_pass_through = false;

			this->branch_index = -1;
			for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
				if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
					for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
						if (this->new_input_node_contexts[i_index][0] == this->best_seed_actions[s_index]) {
							this->branch_index = s_index;
							break;
						}
					}
				} else {
					for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
						if (this->new_input_node_contexts[i_index][0] == this->best_seed_scopes[s_index]) {
							this->branch_index = s_index;
							break;
						}
					}
				}
			}

			for (int i_index = 0; i_index < (int)this->scope_histories.size(); i_index++) {
				delete this->scope_histories[i_index];
			}
			this->scope_histories.clear();
			this->target_val_histories.clear();

			Scope* parent_scope = (Scope*)this->scope_context;

			vector<AbstractNode*> possible_exits;

			if (this->node_context->type == NODE_TYPE_ACTION
					&& ((ActionNode*)this->node_context)->next_node == NULL) {
				possible_exits.push_back(NULL);
			}

			AbstractNode* starting_node;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = info_branch_node->branch_next_node;
					} else {
						starting_node = info_branch_node->original_next_node;
					}
				}
				break;
			}

			parent_scope->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->curr_back_exit_next_node = possible_exits[random_index];

			uniform_int_distribution<int> uniform_distribution(0, 1);
			geometric_distribution<int> geometric_distribution(0.5);
			int new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);

			uniform_int_distribution<int> default_distribution(0, 3);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool default_to_action = true;
				if (default_distribution(generator) != 0) {
					ScopeNode* new_scope_node = create_existing(parent_scope);
					if (new_scope_node != NULL) {
						this->curr_back_step_types.push_back(STEP_TYPE_SCOPE);
						this->curr_back_actions.push_back(NULL);

						this->curr_back_scopes.push_back(new_scope_node);

						default_to_action = false;
					}
				}

				if (default_to_action) {
					this->curr_back_step_types.push_back(STEP_TYPE_ACTION);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->curr_back_actions.push_back(new_action_node);

					this->curr_back_scopes.push_back(NULL);
				}
			}

			this->curr_back_score = 0.0;
			this->best_back_score = numeric_limits<double>::lowest();

			this->state = SEED_EXPERIMENT_STATE_EXPLORE_BACK;
			this->state_iter = 0;
			this->sub_state_iter = 0;
			this->explore_iter = 0;
		}
	}
}
