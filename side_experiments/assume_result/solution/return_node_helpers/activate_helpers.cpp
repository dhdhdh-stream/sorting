#include "return_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "new_action_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void ReturnNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, pair<int,int>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			Minesweeper* minesweeper = (Minesweeper*)problem;
			minesweeper->current_x = it->second.first;
			minesweeper->current_y = it->second.second;
		}
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
		if (it == run_helper.nodes_seen.end()) {
			run_helper.nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	} else if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories.back()->experiment == this->parent->new_action_experiment) {
		context.back().nodes_seen.push_back({this, false});
	}
	Minesweeper* minesweeper = (Minesweeper*)problem;
	context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
