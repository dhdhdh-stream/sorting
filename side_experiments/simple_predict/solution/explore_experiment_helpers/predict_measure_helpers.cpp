#include "explore_experiment.h"

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
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 10;
#else
const int MEASURE_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

void ExploreExperiment::predict_measure_check_activate(vector<double>& obs,
													   ExploreExperimentHistory* history,
													   SolutionWrapper* wrapper) {
	if (wrapper->diversity_index == this->diversity_index) {
		this->num_instances_until_target--;
		if (!history->has_explore
				&& this->num_instances_until_target <= 0) {
			history->has_explore = true;

			bool is_branch;
			this->existing_network->activate(obs);
			double existing_predicted = this->existing_network->output->acti_vals[0];
			this->new_network->activate(obs);
			double new_predicted = this->new_network->output->acti_vals[0];
			if (new_predicted >= existing_predicted) {
				is_branch = true;
			} else {
				is_branch = false;
			}

			#if defined(MDEBUG) && MDEBUG
			if (wrapper->curr_run_seed%2 == 0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
			wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
			#endif /* MDEBUG */

			if (is_branch) {
				history->has_predict = true;
				history->existing_predicted = existing_predicted;

				ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
				new_experiment_state->step_index = 0;
				wrapper->experiment_context.back() = new_experiment_state;
			}
		}
	}
}

void ExploreExperiment::predict_measure_step(vector<double>& obs,
											 int& action,
											 bool& is_next,
											 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;
		} else {
			ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
				this->best_indexes[experiment_state->step_index]];
			generic_scope_node->experiment_step(obs,
												action,
												is_next,
												wrapper);
		}
	}
}

void ExploreExperiment::predict_measure_callback(vector<double>& obs,
												 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	int action = this->best_indexes[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];
	generic_action_node->experiment_step_callback(obs,
												  wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::predict_measure_exit_step(vector<double>& obs,
												  SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
		this->best_indexes[experiment_state->step_index]];
	generic_scope_node->experiment_exit_step(obs,
											 wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::predict_measure_backprop(double target_val,
												 ExploreExperimentHistory* history,
												 SolutionWrapper* wrapper,
												 bool& is_add) {
	if (wrapper->diversity_index == this->diversity_index) {
		double average_instances_per_hit;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				average_instances_per_hit = noop_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				average_instances_per_hit = action_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				average_instances_per_hit = scope_node->average_instances_per_hit;
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					average_instances_per_hit = branch_node->branch_average_instances_per_hit;
				} else {
					average_instances_per_hit = branch_node->original_average_instances_per_hit;
				}
			}
			break;
		}
		uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
		this->num_instances_until_target = until_distribution(generator);

		if (history->has_predict) {
			this->sum_improvement += target_val - history->existing_predicted;

			wrapper->new_since_update++;

			this->state_iter++;
			if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (this->sum_improvement > 0.0 || rand()%2 == 0) {
				#else
				if (this->sum_improvement > 0.0) {
				#endif /* MDEBUG */
					is_add = true;

					add(true,
						wrapper);
				} else {
					double average_instances_per_hit;
					switch (this->node_context->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)this->node_context;
							average_instances_per_hit = noop_node->average_instances_per_hit;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							average_instances_per_hit = action_node->average_instances_per_hit;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;
							average_instances_per_hit = scope_node->average_instances_per_hit;
						}
						break;
					default:
					// case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								average_instances_per_hit = branch_node->branch_average_instances_per_hit;
							} else {
								average_instances_per_hit = branch_node->original_average_instances_per_hit;
							}
						}
						break;
					}
					uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
					this->num_instances_until_target = until_distribution(generator);

					this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
					this->state_iter = 0;
				}
			}
		}
	}
}
