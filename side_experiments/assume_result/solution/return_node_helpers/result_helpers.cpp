#include "return_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ReturnNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	bool is_branch = false;
	if (this->previous_location != NULL) {
		map<AbstractNode*, vector<double>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			vector<double> world_location = problem_type->relative_to_world(
				it->second, this->location);
			problem->return_to_location(world_location);

			is_branch = true;
		}
	}

	if (is_branch) {
		curr_node = this->passed_next_node;
	} else {
		curr_node = this->skipped_next_node;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, is_branch});
		if (it == run_helper.nodes_seen.end()) {
			run_helper.nodes_seen[{this, is_branch}] = 1;
		} else {
			it->second++;
		}
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->result_activate(
			this,
			is_branch,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
