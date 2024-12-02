#include "solution_helpers.h"

#include "abstract_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

const int SET_ITERS = 100;

void set_experiments(Solution* parent_solution) {
	for (int iter_index = 0; iter_index < 100; iter_index++) {
		run_index++;

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> context;
		bool experiment_seen = false;
		map<pair<AbstractNode*,bool>, int> nodes_seen;
		parent_solution->scopes[0]->set_activate(
			problem,
			context,
			run_helper,
			experiment_seen,
			nodes_seen);

		if (!experiment_seen) {
			AbstractNode* explore_node;
			bool explore_is_branch;
			uniform_int_distribution<int> even_distribution(0, 1);
			if (even_distribution(generator) == 0) {
				uniform_int_distribution<int> explore_node_distribution(0, nodes_seen.size()-1);
				int explore_node_index = explore_node_distribution(generator);
				map<pair<AbstractNode*,bool>, int>::iterator it = next(nodes_seen.begin(), explore_node_index);
				explore_node = it->first.first;
				explore_is_branch = it->first.second;
			} else {
				int sum_count = 0;
				for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
						it != nodes_seen.end(); it++) {
					sum_count += it->second;
				}
				uniform_int_distribution<int> random_distribution(1, sum_count);
				int random_index = random_distribution(generator);
				for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
						it != nodes_seen.end(); it++) {
					random_index -= it->second;
					if (random_index <= 0) {
						explore_node = it->first.first;
						explore_is_branch = it->first.second;
						break;
					}
				}
			}
			/**
			 * - don't weigh based on number of nodes within scope
			 *   - can get trapped by small useless scopes
			 *     - may be good for certain decision heavy scopes to have lots of nodes
			 */

			explore_node->is_experiment = true;
			explore_node->experiment_is_branch = explore_is_branch;
		}
	}
}
