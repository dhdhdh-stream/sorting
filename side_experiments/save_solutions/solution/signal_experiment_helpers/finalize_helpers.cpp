#include "signal_experiment.h"

#include "scope.h"
#include "signal.h"
#include "signal_instance.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void SignalExperiment::add(SolutionWrapper* wrapper) {
	map<int, Signal*>::iterator it = wrapper->signals.find(this->scope_context_id);
	if (it == wrapper->signals.end()) {
		it = wrapper->signals.insert({this->scope_context_id, new Signal()}).first;
	}

	it->second->signal_pre_actions = this->pre_actions;
	it->second->signal_post_actions = this->post_actions;
	for (int s_index = 0; s_index < (int)it->second->instances.size(); s_index++) {
		delete it->second->instances[s_index];
	}
	it->second->instances = this->instances;
	this->instances.clear();
	it->second->default_guess = this->default_guess;

	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		delete wrapper->solution->existing_scope_histories[h_index];
	}
	wrapper->solution->existing_scope_histories.clear();
	wrapper->solution->existing_target_val_histories.clear();

	wrapper->solution->clean();
}
