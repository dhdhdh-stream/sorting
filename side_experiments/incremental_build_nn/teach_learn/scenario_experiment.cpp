#include "scenario_experiment.h"

#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "scenario.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

const int LEARN_NUM_DATAPOINTS = 1000;

const int LEARN_ITERS = 40;

ScenarioExperiment::ScenarioExperiment(Scenario* scenario_type) {
	this->scenario_type = scenario_type;

	this->scope = new Scope();

	this->scope->id = solution->scope_counter;
	solution->scope_counter++;

	this->scope->name = this->scenario_type->get_name();

	this->scope->node_counter = 0;

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->parent = this->scope;
	new_noop_action_node->id = this->scope->node_counter;
	this->scope->node_counter++;
	new_noop_action_node->action = Action(ACTION_NOOP);
	this->scope->nodes[new_noop_action_node->id] = new_noop_action_node;

	this->scope->starting_node_id = new_noop_action_node->id;
	this->scope->starting_node = new_noop_action_node;

	vector<int> types;
	vector<Action> actions;
	vector<string> scopes;
	this->scenario_type->get_attention(types,
									   actions,
									   scopes);

	for (int a_index = 0; a_index < (int)types.size(); a_index++) {
		if (types[a_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();

			new_action_node->parent = this->scope;
			new_action_node->id = this->scope->node_counter;
			this->scope->node_counter++;
			this->scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = actions[a_index];
		} else {
			ScopeNode* new_scope_node = new ScopeNode();

			new_scope_node->parent = this->scope;
			new_scope_node->id = this->scope->node_counter;
			this->scope->node_counter++;
			this->scope->nodes[new_scope_node->id] = new_scope_node;

			Scope* scope;
			for (map<int, Scope*>::iterator it = solution->scopes.begin();
					it != solution->scopes.end(); it++) {
				if (it->second->name == scopes[a_index]) {
					scope = it->second;
				}
			}
			new_scope_node->scope = scope;
		}
	}

	for (int n_index = 0; n_index < (int)this->scope->nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)this->scope->nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = this->scope->nodes[n_index+1];
		}

		if (this->scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->scope->nodes[n_index];
			action_node->next_node_id = next_node_id;
			action_node->next_node = next_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->scope->nodes[n_index];
			scope_node->next_node_id = next_node_id;
			scope_node->next_node = next_node;
		}
	}

	this->network = NULL;

	this->scope_histories.reserve(LEARN_NUM_DATAPOINTS);
	this->is_sequence_histories.reserve(LEARN_NUM_DATAPOINTS);

	this->state = SCENARIO_EXPERIMENT_STATE_LEARN_LINEAR;
	this->state_iter = 0;
	this->sub_state_iter = 0;
}

ScenarioExperiment::~ScenarioExperiment() {
	delete this->scenario_type;
}

void ScenarioExperiment::activate(Scenario* scenario,
								  RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(this->scope);
	context.back().scope_history = root_history;

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	this->scope->activate(scenario->problem,
						  context,
						  exit_depth,
						  exit_node,
						  run_helper,
						  root_history);

	this->scope_histories.push_back(root_history);
}

void ScenarioExperiment::backprop(bool is_sequence) {
	this->is_sequence_histories.push_back(is_sequence);

	if ((int)this->is_sequence_histories.size() >= LEARN_NUM_DATAPOINTS) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
							   this->scope_histories.back());
		/**
		 * - simply always use last ScopeHistory
		 */

		if (this->state == SCENARIO_EXPERIMENT_STATE_LEARN_LINEAR) {
			int is_sequence_count = 0;
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				if (this->is_sequence_histories[d_index]) {
					is_sequence_count++;
				}
			}
			this->is_sequence_average = 2.0 * (double)is_sequence_count / (double)LEARN_NUM_DATAPOINTS - 1.0;

			double sum_variance = 0.0;
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				if (this->is_sequence_histories[d_index]) {
					sum_variance += (1.0 - this->is_sequence_average) * (1.0 - this->is_sequence_average);
				} else {
					sum_variance += (-1.0 - this->is_sequence_average) * (-1.0 - this->is_sequence_average);
				}
			}
			double is_sequence_variance = sum_variance / LEARN_NUM_DATAPOINTS;

			int num_obs = min(LINEAR_NUM_OBS, (int)possible_scope_contexts.size());

			vector<int> remaining_indexes(possible_scope_contexts.size());
			for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
				remaining_indexes[p_index] = p_index;
			}
			for (int o_index = 0; o_index < num_obs; o_index++) {
				uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
				int rand_index = distribution(generator);

				this->input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
				this->input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}

			Eigen::MatrixXd inputs(LEARN_NUM_DATAPOINTS, num_obs);

			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
					action_node->hook_indexes.push_back(i_index);
					action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
					branch_node->hook_indexes.push_back(i_index);
					branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				}
			}
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				input_vals_helper(scope_context,
								  node_context,
								  input_vals,
								  this->scope_histories[d_index]);

				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					inputs(d_index, i_index) = input_vals[i_index];
				}
			}
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
					action_node->hook_indexes.clear();
					action_node->hook_scope_contexts.clear();
					action_node->hook_node_contexts.clear();
				} else {
					BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
					branch_node->hook_indexes.clear();
					branch_node->hook_scope_contexts.clear();
					branch_node->hook_node_contexts.clear();
				}
			}

			Eigen::VectorXd outputs(LEARN_NUM_DATAPOINTS);
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				if (this->is_sequence_histories[d_index]) {
					outputs(d_index) = 1.0 - this->is_sequence_average;
				} else {
					outputs(d_index) = -1.0 - this->is_sequence_average;
				}
			}

			Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			this->linear_weights = vector<double>(this->input_scope_contexts.size());
			double standard_deviation = sqrt(is_sequence_variance);
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				double sum_impact_size = 0.0;
				for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
					sum_impact_size += abs(inputs(d_index, i_index));
				}
				double average_impact = sum_impact_size / LEARN_NUM_DATAPOINTS;
				if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * standard_deviation) {
					weights(i_index) = 0.0;
				}
				this->linear_weights[i_index] = weights(i_index);
			}

			Eigen::VectorXd predicted_scores = inputs * weights;
			Eigen::VectorXd diffs = outputs - predicted_scores;
			vector<double> network_target_vals(LEARN_NUM_DATAPOINTS);
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				network_target_vals[d_index] = diffs(d_index);
			}

			vector<double> misguesses(LEARN_NUM_DATAPOINTS);
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				misguesses[d_index] = diffs(d_index) * diffs(d_index);
			}

			double sum_misguesses = 0.0;
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				sum_misguesses += misguesses[d_index];
			}
			this->average_misguess = sum_misguesses / LEARN_NUM_DATAPOINTS;

			double sum_misguess_variances = 0.0;
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				sum_misguess_variances += (misguesses[d_index] - this->average_misguess) * (misguesses[d_index] - this->average_misguess);
			}
			this->misguess_variance = sum_misguess_variances / LEARN_NUM_DATAPOINTS;

			for (int d_index = 0; d_index < (int)this->scope_histories.size(); d_index++) {
				delete this->scope_histories[d_index];
			}
			this->scope_histories.clear();
			this->is_sequence_histories.clear();

			this->scope_histories.reserve(LEARN_NUM_DATAPOINTS);
			this->is_sequence_histories.reserve(LEARN_NUM_DATAPOINTS);

			this->state = SCENARIO_EXPERIMENT_STATE_LEARN_NETWORK;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			vector<vector<vector<double>>> network_inputs(LEARN_NUM_DATAPOINTS);
			vector<double> network_target_vals(LEARN_NUM_DATAPOINTS);

			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
					action_node->hook_indexes.push_back(i_index);
					action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
					branch_node->hook_indexes.push_back(i_index);
					branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				}
			}
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				input_vals_helper(scope_context,
								  node_context,
								  input_vals,
								  this->scope_histories[d_index]);

				double linear_impact = 0.0;
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					linear_impact += input_vals[i_index] * this->linear_weights[i_index];
				}
				if (this->is_sequence_histories[d_index]) {
					network_target_vals[d_index] = 1.0 - this->is_sequence_average - linear_impact;
				} else {
					network_target_vals[d_index] = -1.0 - this->is_sequence_average - linear_impact;
				}

				network_inputs[d_index] = vector<vector<double>>((int)this->network_input_indexes.size());
				for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
					network_inputs[d_index][i_index] = vector<double>((int)this->network_input_indexes[i_index].size());
					for (int s_index = 0; s_index < (int)this->network_input_indexes[i_index].size(); s_index++) {
						network_inputs[d_index][i_index][s_index] = input_vals[this->network_input_indexes[i_index][s_index]];
					}
				}
			}
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
					action_node->hook_indexes.clear();
					action_node->hook_scope_contexts.clear();
					action_node->hook_node_contexts.clear();
				} else {
					BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
					branch_node->hook_indexes.clear();
					branch_node->hook_scope_contexts.clear();
					branch_node->hook_node_contexts.clear();
				}
			}

			int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
			vector<vector<Scope*>> test_network_input_scope_contexts;
			vector<vector<AbstractNode*>> test_network_input_node_contexts;

			vector<int> remaining_indexes(possible_scope_contexts.size());
			for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
				remaining_indexes[p_index] = p_index;
			}
			for (int i_index = 0; i_index < num_new_input_indexes; i_index++) {
				uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
				int rand_index = distribution(generator);

				test_network_input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
				test_network_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}

			Network* test_network;
			if (this->network == NULL) {
				test_network = new Network(num_new_input_indexes);
			} else {
				test_network = new Network(this->network);

				uniform_int_distribution<int> increment_above_distribution(0, 3);
				if (increment_above_distribution(generator) == 0) {
					test_network->increment_above(num_new_input_indexes);
				} else {
					test_network->increment_side(num_new_input_indexes);
				}
			}

			for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
				if (test_network_input_node_contexts[t_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)test_network_input_node_contexts[t_index].back();
					action_node->hook_indexes.push_back(t_index);
					action_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					action_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
					branch_node->hook_indexes.push_back(t_index);
					branch_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					branch_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
				}
			}
			for (int d_index = 0; d_index < LEARN_NUM_DATAPOINTS; d_index++) {
				vector<double> test_input_vals(num_new_input_indexes, 0.0);

				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				input_vals_helper(scope_context,
								  node_context,
								  test_input_vals,
								  this->scope_histories[d_index]);

				network_inputs[d_index].push_back(test_input_vals);
			}
			for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
				if (test_network_input_node_contexts[t_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)test_network_input_node_contexts[t_index].back();
					action_node->hook_indexes.push_back(t_index);
					action_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					action_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
					branch_node->hook_indexes.push_back(t_index);
					branch_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					branch_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
				}
			}

			train_network(network_inputs,
						  network_target_vals,
						  test_network_input_scope_contexts,
						  test_network_input_node_contexts,
						  test_network);

			double average_misguess;
			double misguess_variance;
			measure_network(network_inputs,
							network_target_vals,
							test_network,
							average_misguess,
							misguess_variance);

			double improvement = this->average_misguess - average_misguess;
			double standard_deviation = (sqrt(this->misguess_variance) + sqrt(misguess_variance)) / 2.0;
			double t_score = improvement / (standard_deviation / sqrt(solution->curr_num_datapoints * TEST_SAMPLES_PERCENTAGE));
			if (t_score > 2.326) {
				optimize_network(network_inputs,
								 network_target_vals,
								 test_network);

				double final_average_misguess;
				double final_misguess_variance;
				measure_network(network_inputs,
								network_target_vals,
								test_network,
								final_average_misguess,
								final_misguess_variance);

				vector<int> new_input_indexes;
				for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
					int index = -1;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						if (test_network_input_scope_contexts[t_index] == this->input_scope_contexts[i_index]
								&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]) {
							index = i_index;
							break;
						}
					}
					if (index == -1) {
						this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
						this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);

						this->linear_weights.push_back(0.0);
						this->linear_weights.push_back(0.0);

						index = this->input_scope_contexts.size()-1;
					}
					new_input_indexes.push_back(index);
				}
				this->network_input_indexes.push_back(new_input_indexes);

				if (this->network != NULL) {
					delete this->network;
				}
				this->network = test_network;

				this->average_misguess = final_average_misguess;
				this->misguess_variance = final_misguess_variance;
			} else {
				delete test_network;
			}

			for (int d_index = 0; d_index < (int)this->scope_histories.size(); d_index++) {
				delete this->scope_histories[d_index];
			}
			this->scope_histories.clear();
			this->is_sequence_histories.clear();

			this->sub_state_iter++;
			if (this->sub_state_iter > LEARN_ITERS) {
				AbstractNode* attention_last_node = this->scope->nodes[this->scope->nodes.size()-1];

				BranchNode* new_branch_node = new BranchNode();
				new_branch_node->parent = this->scope;
				new_branch_node->id = this->scope->node_counter;
				this->scope->node_counter++;
				this->scope->nodes[new_branch_node->id] = new_branch_node;

				new_branch_node->scope_context_ids = vector<int>{this->scope->id};
				new_branch_node->scope_context = vector<Scope*>{this->scope};
				new_branch_node->node_context_ids = vector<int>{new_branch_node->id};
				new_branch_node->node_context = vector<AbstractNode*>{new_branch_node};

				new_branch_node->is_pass_through = false;

				new_branch_node->original_average_score = 0.0;
				new_branch_node->branch_average_score = this->is_sequence_average;

				vector<int> input_mapping(this->input_scope_contexts.size(), -1);
				for (int i_index = 0; i_index < (int)this->linear_weights.size(); i_index++) {
					if (this->linear_weights[i_index] != 0.0) {
						if (input_mapping[i_index] == -1) {
							input_mapping[i_index] = (int)new_branch_node->input_scope_contexts.size();
							new_branch_node->input_scope_contexts.push_back(this->input_scope_contexts[i_index]);
							vector<int> scope_context_ids;
							for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
								scope_context_ids.push_back(this->input_scope_contexts[i_index][c_index]->id);
							}
							new_branch_node->input_scope_context_ids.push_back(scope_context_ids);
							new_branch_node->input_node_contexts.push_back(this->input_node_contexts[i_index]);
							vector<int> node_context_ids;
							for (int c_index = 0; c_index < (int)this->input_node_contexts[i_index].size(); c_index++) {
								node_context_ids.push_back(this->input_node_contexts[i_index][c_index]->id);
							}
							new_branch_node->input_node_context_ids.push_back(node_context_ids);
						}
					}
				}
				for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
					for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
						int original_index = this->network_input_indexes[i_index][v_index];
						if (input_mapping[original_index] == -1) {
							input_mapping[original_index] = (int)new_branch_node->input_scope_contexts.size();
							new_branch_node->input_scope_contexts.push_back(this->input_scope_contexts[original_index]);
							vector<int> scope_context_ids;
							for (int c_index = 0; c_index < (int)this->input_scope_contexts[original_index].size(); c_index++) {
								scope_context_ids.push_back(this->input_scope_contexts[original_index][c_index]->id);
							}
							new_branch_node->input_scope_context_ids.push_back(scope_context_ids);
							new_branch_node->input_node_contexts.push_back(this->input_node_contexts[original_index]);
							vector<int> node_context_ids;
							for (int c_index = 0; c_index < (int)this->input_node_contexts[original_index].size(); c_index++) {
								node_context_ids.push_back(this->input_node_contexts[original_index][c_index]->id);
							}
							new_branch_node->input_node_context_ids.push_back(node_context_ids);
						}
					}
				}

				for (int i_index = 0; i_index < (int)this->linear_weights.size(); i_index++) {
					if (this->linear_weights[i_index] != 0.0) {
						new_branch_node->linear_branch_input_indexes.push_back(input_mapping[i_index]);
						new_branch_node->linear_branch_weights.push_back(this->linear_weights[i_index]);
					}
				}

				new_branch_node->original_network = NULL;
				for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
					vector<int> input_indexes;
					for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
						input_indexes.push_back(input_mapping[this->network_input_indexes[i_index][v_index]]);
					}
					new_branch_node->branch_network_input_indexes.push_back(input_indexes);
				}
				new_branch_node->branch_network = this->network;
				this->network = NULL;

				if (attention_last_node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)attention_last_node;

					action_node->next_node_id = new_branch_node->id;
					action_node->next_node = new_branch_node;
				} else {
					ScopeNode* scope_node = (ScopeNode*)attention_last_node;

					scope_node->next_node_id = new_branch_node->id;
					scope_node->next_node = new_branch_node;
				}

				vector<int> types;
				vector<Action> actions;
				vector<string> scopes;
				this->scenario_type->get_sequence(types,
												  actions,
												  scopes);

				for (int a_index = 0; a_index < (int)types.size(); a_index++) {
					if (types[a_index] == STEP_TYPE_ACTION) {
						ActionNode* new_action_node = new ActionNode();

						new_action_node->parent = this->scope;
						new_action_node->id = this->scope->node_counter;
						this->scope->node_counter++;
						this->scope->nodes[new_action_node->id] = new_action_node;

						new_action_node->action = actions[a_index];
					} else {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = this->scope;
						new_scope_node->id = this->scope->node_counter;
						this->scope->node_counter++;
						this->scope->nodes[new_scope_node->id] = new_scope_node;

						Scope* scope;
						for (map<int, Scope*>::iterator it = solution->scopes.begin();
								it != solution->scopes.end(); it++) {
							if (it->second->name == scopes[a_index]) {
								scope = it->second;
							}
						}
						new_scope_node->scope = scope;
					}
				}

				new_branch_node->original_next_node_id = -1;
				new_branch_node->original_next_node = NULL;

				new_branch_node->branch_next_node_id = new_branch_node->id + 1;
				new_branch_node->branch_next_node = this->scope->nodes[new_branch_node->id + 1];

				for (int a_index = 0; a_index < (int)types.size(); a_index++) {
					int next_node_id;
					AbstractNode* next_node;
					if (a_index == (int)actions.size()-1) {
						next_node_id = -1;
						next_node = NULL;
					} else {
						next_node_id = new_branch_node->id + 1 + a_index + 1;
						next_node = this->scope->nodes[new_branch_node->id + 1 + a_index + 1];
					}

					if (this->scope->nodes[new_branch_node->id + 1 + a_index]->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->scope->nodes[new_branch_node->id + 1 + a_index];
						action_node->next_node_id = next_node_id;
						action_node->next_node = next_node;
					} else {
						ScopeNode* scope_node = (ScopeNode*)this->scope->nodes[new_branch_node->id + 1 + a_index];
						scope_node->next_node_id = next_node_id;
						scope_node->next_node = next_node;
					}
				}

				solution->scopes[this->scope->id] = this->scope;

				this->state = SCENARIO_EXPERIMENT_STATE_DONE;
			} else {
				this->scope_histories.reserve(LEARN_NUM_DATAPOINTS);
				this->is_sequence_histories.reserve(LEARN_NUM_DATAPOINTS);
			}
		}
	}
}
