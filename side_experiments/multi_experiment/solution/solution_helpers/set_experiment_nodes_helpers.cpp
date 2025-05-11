/**
 * - makes it such that original paths will not be correlated with experiments
 *   - because experiments are likely negative, would likely frequently give new extra score
 *     - which can be especially bad when damaging news are added
 */

#include "solution_helpers.h"

#include <iostream>

#include "branch_experiment.h"
#include "branch_node.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

const int NODES_PER_EXPERIMENT = 20;

void create_experiment(AbstractNode* experiment_node,
					   int improvement_iter) {
	/**
	 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
	 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
	 *     - like tessellation, but have to get both the shape and the pattern correct
	 *       - and PassThroughExperiments help with both
	 */
	// if (improvement_iter == 0) {
	if (false) {
		// CommitExperiment* new_experiment = new CommitExperiment(
		// 	experiment_node->parent,
		// 	experiment_node,
		// 	experiment_node->experiment_is_branch,
		// 	experiment_node->experiment_exit_next_node);
		// experiment_node->experiment = experiment_node;
	} else {
		uniform_int_distribution<int> pass_through_distribution(0, 1);
		if (pass_through_distribution(generator) == 0
				&& improvement_iter > 3) {
			PassThroughExperiment* new_experiment = new PassThroughExperiment(
				experiment_node->parent,
				experiment_node,
				experiment_node->experiment_is_branch,
				experiment_node->experiment_exit_next_node);
			experiment_node->experiment = new_experiment;
		} else {
			BranchExperiment* new_experiment = new BranchExperiment(
				experiment_node->parent,
				experiment_node,
				experiment_node->experiment_is_branch,
				experiment_node->experiment_exit_next_node);
			experiment_node->experiment = new_experiment;
		}
	}
}

void set_experiment_nodes() {
	run_index++;

	Problem* problem = problem_type->get_problem();

	RunHelper run_helper;

	#if defined(MDEBUG) && MDEBUG
	run_helper.starting_run_seed = run_index;
	run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
	solution->scopes[0]->experiment_activate(
			problem,
			run_helper,
			scope_history);

	vector<AbstractNode*> nodes(scope_history->node_histories.size());
	vector<bool> is_branches(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		nodes[it->second->index] = it->second->node;
		if (it->second->node->type == NODE_TYPE_BRANCH) {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
			is_branches[it->second->index] = branch_node_history->is_branch;
		} else {
			is_branches[it->second->index] = false;
		}
	}

	int max_num_experiments = ((int)nodes.size() + NODES_PER_EXPERIMENT-1) / NODES_PER_EXPERIMENT;

	uniform_int_distribution<int> num_experiments_distribution(1, max_num_experiments);
	int num_experiments = num_experiments_distribution(generator);

	double nodes_per = (int)nodes.size() / (double)num_experiments;

	nodes.push_back(NULL);

	vector<int> starts(num_experiments);
	vector<int> ends(num_experiments);
	starts[0] = 0;
	for (int e_index = 0; e_index < num_experiments-1; e_index++) {
		int divider = nodes_per * (e_index + 1);
		starts[e_index + 1] = divider;
		ends[e_index] = divider;
	}
	ends[num_experiments-1] = (int)nodes.size() - 1;

	for (int e_index = 0; e_index < num_experiments; e_index++) {
		uniform_int_distribution<int> start_distribution(starts[e_index], ends[e_index]-1);
		uniform_int_distribution<int> end_distribution(starts[e_index], ends[e_index]);
		int start_index;
		int end_index;
		while (true) {
			start_index = start_distribution(generator);
			end_index = end_distribution(generator);
			if (start_index < end_index) {
				break;
			}
		}

		nodes[start_index]->experiment_is_branch = is_branches[start_index];
		nodes[start_index]->experiment_exit_next_node = nodes[end_index];

		create_experiment(nodes[start_index],
						  0);
	}

	delete scope_history;

	delete problem;
}
