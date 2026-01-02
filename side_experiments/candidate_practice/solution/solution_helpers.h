#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

class AbstractNode;
class BranchExperiment;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;
class Tunnel;

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

void set_tunnel(SolutionWrapper* wrapper);
void update_tunnel_try_history(SolutionWrapper* wrapper);

void measure_tunnel_vals_helper(ScopeHistory* scope_history,
								Scope* tunnel_parent,
								int tunnel_index,
								double& sum_vals);
void measure_tunnel_vals_helper(ScopeHistory* scope_history);

Scope* create_new_scope(Scope* scope_context);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

void find_potential_tunnels(std::vector<ScopeHistory*>& starting_scope_histories,
							std::vector<ScopeHistory*>& ending_scope_histories);

#endif /* SOLUTION_HELPERS_H */