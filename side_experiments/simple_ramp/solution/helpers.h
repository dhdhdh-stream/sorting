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

void create_experiment(SolutionWrapper* wrapper);

void fetch_histories_helper(ScopeHistory* scope_history,
							ObsNode* node_context,
							std::vector<std::vector<double>>& obs_histories);

Scope* create_new_scope(Scope* scope_context);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

#endif /* HELPERS_H */