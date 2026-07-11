#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class BranchExperiment;
class BranchNode;
class InitNetwork;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

void gather_dependencies_helper(ScopeHistory* scope_history,
								std::vector<int>& curr_context,
								std::vector<int>& curr_index,
								int& count,
								std::vector<int>& dependency,
								std::vector<int>& index);
void set_dependency_helper(Scope* scope,
						   std::vector<int>& dependency,
						   int l_index,
						   AbstractExperiment* experiment);
void clear_dependency_helper(Scope* scope,
							 std::vector<int>& dependency,
							 int l_index,
							 AbstractExperiment* experiment);
void fetch_dependency_helper(ScopeHistory* scope_history,
							 std::vector<int>& dependency,
							 int l_index,
							 bool& is_hit,
							 std::vector<double>& state,
							 std::vector<double>& obs);
void add_dependency_helper(Scope* scope,
						   std::vector<Scope*>& init_network_scope_context,
						   std::vector<int>& dependency,
						   int l_index,
						   InitNetwork* init_network);
bool match_dependency_helper(SolutionWrapper* wrapper,
							 std::vector<Scope*>& scope_contexts,
							 std::vector<int>& node_contexts);

void update_helper(ScopeHistory* scope_history);
void update_helper(double target_val,
				   SolutionWrapper* wrapper);

double measure_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */