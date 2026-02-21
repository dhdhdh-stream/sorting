#ifndef SIGNAL_HELPERS_H
#define SIGNAL_HELPERS_H

#include <vector>

#include "signal_input.h"

class Scope;
class ScopeHistory;
class SolutionWrapper;

void set_potential_inputs(Scope* scope);

/**
 * - explore_index = node_histories.size()-1
 */
void fetch_input_helper(ScopeHistory* scope_history,
						SignalInput& input,
						std::vector<int>& explore_index,
						int l_index,
						double& val,
						bool& is_on);

void signal_add_sample(ScopeHistory* scope_history,
					   std::vector<int>& explore_index,
					   double target_val,
					   bool is_existing,
					   SolutionWrapper* wrapper);

void update_signal(Scope* scope,
				   SolutionWrapper* wrapper);

#endif /* SIGNAL_HELPERS_H */