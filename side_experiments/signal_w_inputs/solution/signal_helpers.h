#ifndef SIGNAL_HELPERS_H
#define SIGNAL_HELPERS_H

#include <vector>

#include "signal_input.h"

class Scope;
class ScopeHistory;
class SolutionWrapper;

void set_pre_signal_potential_inputs(Scope* scope);
void set_post_signal_potential_inputs(Scope* scope);

/**
 * - explore_index = node_histories.size()-1
 */
void fetch_input_helper(ScopeHistory* scope_history,
						SignalInput& input,
						std::vector<int>& explore_index,
						int l_index,
						double& val,
						bool& is_on);

/**
 * - for pre, simply add 1 sample per run/target_val
 *   - so then simply use explore_index of that run
 */
void pre_signal_add_sample(ScopeHistory* scope_history,
						   std::vector<int>& explore_index,
						   double target_val,
						   SolutionWrapper* wrapper);
void post_signal_add_sample(ScopeHistory* scope_history,
							std::vector<int>& explore_index,
							double target_val,
							bool is_existing,
							SolutionWrapper* wrapper);

void update_pre_signal(Scope* scope,
					   SolutionWrapper* wrapper);
void update_post_signal(Scope* scope,
						SolutionWrapper* wrapper);

#endif /* SIGNAL_HELPERS_H */