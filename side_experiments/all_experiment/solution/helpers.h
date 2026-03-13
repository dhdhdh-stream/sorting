#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

class AbstractNode;
class BranchExperiment;
class Network;
class ObsNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_scope_experiment(SolutionWrapper* wrapper);

void create_new_scope(AbstractNode* potential_start_node,
					  AbstractNode* potential_end_node,
					  Scope*& new_scope);
void create_new_scope(Scope* scope_context,
					  Scope*& new_scope);
void outer_create_new_scope(SolutionWrapper* wrapper,
							Scope*& new_scope);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

void measure_vals(SolutionWrapper* wrapper);

// temp
double result_helper(SolutionWrapper* wrapper);

#endif /* HELPERS_H */