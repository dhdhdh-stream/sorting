#include "helpers.h"

#include "constants.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void iter_update_vals_helper(ScopeHistory* scope_history,
							 double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				iter_update_vals_helper(scope_node_history->scope_history,
										target_val);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)h_it->second->node;
				obs_node->sum_vals += target_val;
				obs_node->hit_count++;
			}
			break;
		}
	}
}

void update_vals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				obs_node->update_val();
			}
		}
	}
}

void measure_vals(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	for (int iter_index = 0; iter_index < MEASURE_NUM_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;

		iter_update_vals_helper(wrapper->scope_histories[0],
								target_val);

		wrapper->end();

		delete problem;
	}

	update_vals(wrapper);

	delete problem_type;
}
