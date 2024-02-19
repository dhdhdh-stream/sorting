#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::train_filter_activate(vector<ContextLayer>& context) {
	this->parent->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));
}
