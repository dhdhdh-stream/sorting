#include "scope_node.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::continue_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  int curr_layer,
								  RunHelper& run_helper) {
	this->scope->continue_activate(problem,
								   context,
								   curr_layer,
								   run_helper);

	context.back().node = NULL;

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();
}

void ScopeNode::continue_experiment_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int curr_layer,
		RunHelper& run_helper) {
	this->scope->continue_experiment_activate(
		problem,
		context,
		curr_layer,
		run_helper);

	context.back().node = NULL;

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();
}
