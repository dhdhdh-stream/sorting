#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>

#include "scope.h"

class Solution {
public:
	double average_score;

	std::vector<Scope*> scopes;
	// scopes[0] is root, starts with ACTION_START, don't include in explore

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	Solution(std::ifstream& input_file);
	~Solution();

	void random_run_helper(int scope_id,
						   std::vector<bool>& is_inner_scope,
						   std::vector<int>& existing_scope_ids,
						   std::vector<Action>& actions,
						   std::vector<int>& scope_context,
						   std::vector<int>& node_context,
						   int& early_exit_depth,
						   int& early_exit_node_id,
						   bool& exceeded_depth);
	void new_sequence(std::vector<bool>& is_inner_scope,
					  std::vector<int>& existing_scope_ids,
					  std::vector<Action>& actions,
					  bool can_be_empty);

	void random_run_continuation_helper(int scope_id,
										std::vector<int>& scope_context,
										std::vector<int>& node_context,
										int& early_exit_depth,
										int& early_exit_node_id);
	void random_run_continuation(int explore_node_next_node_id,
								 std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 std::vector<int>& potential_exit_depths,
								 std::vector<int>& potential_next_node_ids);

	void backtrack_for_loop_helper(ScopeHistory* scope_history,
								   int& remaining_length,
								   std::vector<bool>& is_inner_scope,
								   std::vector<int>& existing_scope_ids,
								   std::vector<Action>& actions);
	void backtrack_for_loop(std::vector<ScopeHistory*>& context_histories,
							std::vector<bool>& is_inner_scope,
							std::vector<int>& existing_scope_ids,
							std::vector<Action>& actions);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */