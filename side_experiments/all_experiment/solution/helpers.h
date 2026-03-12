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

void create_new_scope(AbstractNode* potential_start_node,
					  AbstractNode* potential_end_node,
					  Scope*& new_scope);
void create_new_scope(Scope* scope_context,
					  SolutionWrapper* wrapper,
					  Scope*& new_scope,
					  Scope*& parent_scope);
void outer_create_new_scope(Scope* scope_context,
							SolutionWrapper* wrapper,
							Scope*& new_scope,
							Scope*& parent_scope);

void clean_scope(Scope* scope);

// temp
double result_helper(SolutionWrapper* wrapper);

#endif /* HELPERS_H */