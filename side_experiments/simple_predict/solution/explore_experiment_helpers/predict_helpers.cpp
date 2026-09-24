#include "explore_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "predict_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 10;
#else
const int NUM_EXPLORE = 400;
#endif /* MDEBUG */

bool ExploreExperiment::predict_cycle() {
	double predict_surprise = 0.0;

	uniform_int_distribution<int> run_distribution(0, this->existing_obs_histories.size()-1);
	for (int e_index = 0; e_index < NUM_EXPLORE; e_index++) {
		int run_index = run_distribution(generator);

		uniform_int_distribution<int> instance_distribution(0, this->existing_obs_histories[run_index].size()-1);
		int instance_index = instance_distribution(generator);

		this->existing_network->activate(this->existing_obs_histories[run_index][instance_index]);
		double existing_predicted = this->existing_network->output->acti_vals(0);

		vector<int> curr_step_types;
		vector<int> curr_indexes;

		bool exit_is_next;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				if (this->exit_next_node == noop_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				if (this->exit_next_node == action_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				if (this->exit_next_node == scope_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					if (this->exit_next_node == branch_node->branch_next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				} else {
					if (this->exit_next_node == branch_node->original_next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
			}
			break;
		}

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		/**
		 * - num_steps less than exit length on average to reduce solution size
		 */
		if (exit_is_next) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> possible_child_indexes;
		for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
			if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
				possible_child_indexes.push_back(c_index);
			}
		}
		uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
		uniform_int_distribution<int> action_distribution(0, this->scope_context->generic_action_nodes.size()-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool is_scope = false;
			if (possible_child_indexes.size() > 0) {
				if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
					uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
					if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
						is_scope = true;
					}
				} else {
					uniform_int_distribution<int> scope_distribution(0, 1);
					if (scope_distribution(generator) == 0) {
						is_scope = true;
					}
				}
			}
			if (is_scope) {
				curr_step_types.push_back(STEP_TYPE_SCOPE);
				int child_index = possible_child_indexes[child_index_distribution(generator)];
				curr_indexes.push_back(child_index);
			} else {
				curr_step_types.push_back(STEP_TYPE_ACTION);
				curr_indexes.push_back(action_distribution(generator));
			}
		}

		Eigen::VectorXf state = this->existing_state_histories[run_index][instance_index];
		for (int s_index = 0; s_index < (int)curr_step_types.size(); s_index++) {
			if (curr_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* generic_action_node = this->scope_context->generic_action_nodes[curr_indexes[s_index]];
				generic_action_node->predict_network->activate(state);
			} else {
				ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[curr_indexes[s_index]];
				generic_scope_node->predict_network->activate(state);
			}
		}
		AbstractNode* curr_node = this->exit_next_node;
		while (curr_node != NULL) {
			curr_node->predict_step(state,
									curr_node);
		}
		this->scope_context->score_network->activate(state);
		double predicted = this->scope_context->score_network->output->acti_vals(0);

		double curr_surprise = predicted - existing_predicted;
		if (curr_surprise > predict_surprise) {
			predict_surprise = curr_surprise;
			this->best_step_types = curr_step_types;
			this->best_indexes = curr_indexes;
		}
	}

	if (predict_surprise > 0.0) {
		vector<vector<double>> predict_obs_histories;
		vector<double> predict_target_val_histories;
		for (int h_index = 0; h_index < (int)this->existing_obs_histories.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_obs_histories[h_index].size(); i_index++) {
				predict_obs_histories.push_back(this->existing_obs_histories[h_index][i_index]);

				Eigen::VectorXf state = this->existing_state_histories[h_index][i_index];
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNode* generic_action_node = this->scope_context->generic_action_nodes[this->best_indexes[s_index]];
						generic_action_node->predict_network->activate(state);
					} else {
						ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[this->best_indexes[s_index]];
						generic_scope_node->predict_network->activate(state);
					}
				}
				AbstractNode* curr_node = this->exit_next_node;
				while (curr_node != NULL) {
					curr_node->predict_step(state,
											curr_node);
				}
				this->scope_context->score_network->activate(state);
				predict_target_val_histories.push_back(this->scope_context->score_network->output->acti_vals(0));
			}
		}

		if (this->new_network != NULL) {
			delete this->new_network;
		}
		this->new_network = new Network(predict_obs_histories[0].size());
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double hidden_3_average_max_update = 0.0;
		double output_average_max_update = 0.0;

		uniform_int_distribution<int> predict_distribution(0, predict_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_NEW_ITERS.back(); iter_index++) {
			int sample_index = predict_distribution(generator);

			this->new_network->activate(predict_obs_histories[sample_index]);

			double error = predict_target_val_histories[sample_index] - this->new_network->output->acti_vals(0);

			this->new_network->init_backprop(error,
											 hidden_1_average_max_update,
											 hidden_2_average_max_update,
											 hidden_3_average_max_update,
											 output_average_max_update);
		}

		double existing_sum_vals = 0.0;
		int existing_count = 0;
		for (int r_index = 0; r_index < (int)this->existing_obs_histories.size(); r_index++) {
			for (int i_index = 0; i_index < (int)this->existing_obs_histories[r_index].size(); i_index++) {
				this->existing_network->activate(this->existing_obs_histories[r_index][i_index]);
				double existing_predicted = this->existing_network->output->acti_vals[0];
				this->new_network->activate(this->existing_obs_histories[r_index][i_index]);
				double predicted = this->new_network->output->acti_vals[0];
				if (predicted >= existing_predicted) {
					existing_sum_vals += this->existing_target_val_histories[r_index];
					existing_count++;
				}
			}
		}
		double existing_average = existing_sum_vals / (double)existing_count;
		double predict_sum_vals = 0.0;
		int predict_count = 0;
		for (int h_index = 0; h_index < (int)predict_obs_histories.size(); h_index++) {
			this->existing_network->activate(predict_obs_histories[h_index]);
			double existing_predicted = this->existing_network->output->acti_vals[0];
			this->new_network->activate(predict_obs_histories[h_index]);
			double predicted = this->new_network->output->acti_vals[0];
			if (predicted >= existing_predicted) {
				predict_sum_vals += predict_target_val_histories[h_index];
				predict_count++;
			}
		}
		double predict_average = predict_sum_vals / (double)predict_count;
		double average_ratio = (existing_count + predict_count)
			/ ((double)this->existing_obs_histories.size()
				+ (double)predict_obs_histories.size());
		double local_improvement = (predict_average - existing_average) * average_ratio;

		double average_instances_per_run;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				average_instances_per_run = noop_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				average_instances_per_run = action_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				average_instances_per_run = scope_node->average_instances_per_run;
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					average_instances_per_run = branch_node->branch_average_instances_per_run;
				} else {
					average_instances_per_run = branch_node->original_average_instances_per_run;
				}
			}
			break;
		}
		double global_improvement = average_instances_per_run * local_improvement;

		bool is_success = false;
		if (local_improvement > 0.0) {
			if ((int)this->scope_context->predict_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = this->scope_context->predict_last_scores.begin();
						it != this->scope_context->predict_last_scores.end(); it++) {
					if (global_improvement >= *it) {
						num_better_than++;
					}
				}

				double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->predict_last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if ((int)this->scope_context->predict_last_scores.size() >= NUM_LAST_TRACK) {
					this->scope_context->predict_last_scores.pop_front();
				}
				this->scope_context->predict_last_scores.push_back(global_improvement);
			} else {
				this->scope_context->predict_last_scores.push_back(global_improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		return is_success || rand()%4 != 0;
		#else
		return is_success;
		#endif /* MDEBUG */
	} else {
		return false;
	}
}
