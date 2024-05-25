// #include "new_action_experiment.h"

// #include "scope.h"

// using namespace std;

// void NewActionExperiment::successful_activate(
// 		int location_index,
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		NewActionExperimentHistory* history) {
// 	context.push_back(ContextLayer());

// 	context.back().scope = this->scope_context;
// 	context.back().node = NULL;

// 	ScopeHistory* scope_history = new ScopeHistory(this->scope_context);
// 	context.back().scope_history = scope_history;

// 	this->scope_context->new_action_activate(this->starting_node,
// 											 this->included_nodes,
// 											 problem,
// 											 context,
// 											 run_helper,
// 											 scope_history);

// 	delete scope_history;

// 	context.pop_back();

// 	curr_node = this->successful_location_exits[location_index];
// }
