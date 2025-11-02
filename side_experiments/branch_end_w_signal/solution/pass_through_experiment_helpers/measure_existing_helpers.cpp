#include "pass_through_experiment.h"

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_EXISTING_NUM_DATAPOINTS = 20;
#else
const int MEASURE_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void PassThroughExperiment::measure_existing_check_activate(
		SolutionWrapper* wrapper) {
	wrapper->experiment_history->stack_traces.push_back(wrapper->scope_histories);
}

void PassThroughExperiment::measure_existing_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	add_existing_samples(wrapper->scope_histories[0],
						 target_val);

	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		double curr_sum_signals = 0.0;
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			double sum_vals = target_val - wrapper->solution->curr_score;
			int sum_counts = 1;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->consistency_network != NULL) {
					if (!scope_history->signal_initialized) {
						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->consistency_network->activate(inputs);
						scope_history->signal_initialized = true;
						#if defined(MDEBUG) && MDEBUG
						scope_history->consistency_val = 2 * (rand()%2) - 1;
						#else
						scope_history->consistency_val = scope->consistency_network->output->acti_vals[0];
						#endif /* MDEBUG */

						scope->pre_network->activate(scope_history->pre_obs);
						scope_history->pre_val = scope->pre_network->output->acti_vals[0];

						scope->post_network->activate(inputs);
						scope_history->post_val = scope->post_network->output->acti_vals[0];
					}

					sum_vals += (scope_history->post_val - scope_history->pre_val);
					sum_counts++;
				}
			}

			double average_val = sum_vals / sum_counts;

			curr_sum_signals += average_val;
		}
		this->sum_signals += curr_sum_signals / (double)history->stack_traces.size();

		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_EXISTING_NUM_DATAPOINTS) {
			this->existing_score = this->sum_scores / this->state_iter;
			this->existing_signal = this->sum_signals / this->state_iter;

			this->num_explores = 0;

			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (this->node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)this->node_context;
					starting_node = start_node->next_node;
				}
				break;
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
			case NODE_TYPE_BRANCH_END:
				{
					BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;
					starting_node = branch_end_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			geometric_distribution<int> exit_distribution(0.1);
			int random_index;
			while (true) {
				random_index = exit_distribution(generator);
				if (random_index < (int)possible_exits.size()) {
					break;
				}
			}
			this->exit_next_node = possible_exits[random_index];

			uniform_int_distribution<int> new_scope_distribution(0, 1);
			if (new_scope_distribution(generator) == 0) {
				this->new_scope = create_new_scope(this->node_context->parent);
			}
			if (this->new_scope != NULL) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(-1);
				this->scopes.push_back(this->new_scope);
			} else {
				int new_num_steps;
				geometric_distribution<int> geo_distribution(0.3);
				/**
				 * - num_steps less than exit length on average to reduce solution size
				 */
				if (random_index == 0) {
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
						this->step_types.push_back(STEP_TYPE_SCOPE);
						this->actions.push_back(-1);

						int child_index = possible_child_indexes[child_index_distribution(generator)];
						this->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
					} else {
						this->step_types.push_back(STEP_TYPE_ACTION);

						this->actions.push_back(-1);

						this->scopes.push_back(NULL);
					}
				}
			}

			this->total_count = 0;
			this->total_sum_scores = 0.0;

			this->sum_scores = 0.0;
			this->sum_signals = 0.0;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_C1;
			this->state_iter = 0;
		}
	}
}
