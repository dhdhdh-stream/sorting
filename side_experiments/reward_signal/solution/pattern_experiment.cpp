#include "pattern_experiment.h"

#include "scope.h"

using namespace std;

PatternExperiment::PatternExperiment(Scope* scope_context) {
	this->scope_context = scope_context;
}

PatternExperiment::~PatternExperiment() {
	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}

	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		delete this->explore_scope_histories[h_index];
	}
}
