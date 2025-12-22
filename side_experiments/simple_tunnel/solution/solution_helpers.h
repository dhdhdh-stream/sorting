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

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

Scope* create_new_scope(Scope* scope_context);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

bool calc_linear_regression(std::vector<std::vector<double>>& inputs,
							std::vector<double>& outputs,
							std::vector<double>& weights);
double calc_pcc(std::vector<double>& val_1s,
				std::vector<double>& val_2s);

#endif /* SOLUTION_HELPERS_H */