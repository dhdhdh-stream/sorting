#include "solution_helpers.h"

#include "action_network.h"
#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "lstm.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void mimic_helper(ScopeHistory* scope_history,
				  vector<double>& state_vals) {
	vector<AbstractNodeHistory*> ordered_history(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		ordered_history[it->second->index] = it->second;
	}

	for (int h_index = 0; h_index < (int)ordered_history.size(); h_index++) {
		switch (ordered_history[h_index]->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)ordered_history[h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				if (action_node->action.move != ACTION_NOOP) {
					for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
						mimic_memory_cells[s_index]->activate(
							action_node_history->obs_history,
							action_node->action.move + 1,
							state_vals);
					}

					for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
						state_vals[s_index] = mimic_memory_cells[s_index]->memory_val;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)ordered_history[h_index];

				mimic_helper(scope_node_history->scope_history,
							 state_vals);
			}
			break;
		}
	}
}

void mimic(Problem* problem,
		   ScopeHistory* scope_history,
		   int num_steps,
		   vector<Action>& mimic_actions) {
	vector<double> state_vals(MIMIC_NUM_STATES, 0.0);

	{
		ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[0];

		for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
			mimic_memory_cells[s_index]->activate(
				action_node_history->obs_history,
				0,
				state_vals);
		}

		for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
			state_vals[s_index] = mimic_memory_cells[s_index]->memory_val;
		}
	}
	mimic_helper(scope_history,
				 state_vals);

	for (int step_index = 0; step_index < num_steps; step_index++) {
		vector<double> output_state_vals(MIMIC_NUM_STATES);
		for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
			output_state_vals[s_index] = mimic_memory_cells[s_index]->output;
		}

		/**
		 * - don't include ACTION_TERMINATE
		 */
		for (int a_index = 1; a_index < (int)mimic_action_networks.size(); a_index++) {
			mimic_action_networks[a_index]->activate(output_state_vals);
		}

		double sum_probability = 0.0;
		for (int a_index = 1; a_index < (int)mimic_action_networks.size(); a_index++) {
			sum_probability += mimic_action_networks[a_index]->output->acti_vals[0];
		}
		uniform_real_distribution<double> action_distribution(0.0, sum_probability);
		double rand_val = action_distribution(generator);
		Action new_action;
		{
			int a_index = 1;
			while (true) {
				rand_val -= mimic_action_networks[a_index]->output->acti_vals[0];
				if (rand_val <= 0.0) {
					new_action = Action(a_index-1);
				} else {
					a_index++;
				}
			}
		}

		mimic_actions.push_back(new_action);

		problem->perform_action(new_action);

		vector<double> obs = problem->get_observations();

		for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
			mimic_memory_cells[s_index]->activate(
				obs,
				new_action.move + 1,
				state_vals);
		}

		for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
			state_vals[s_index] = mimic_memory_cells[s_index]->memory_val;
		}
	}
}
