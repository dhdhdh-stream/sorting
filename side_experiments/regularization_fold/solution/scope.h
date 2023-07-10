/**
 * - scopes are just a bit of abstraction to try to promote the reuse of ideas
 *   - scopes roughly capture when certain state is relevant, but is extremely imprecise
 *     - the actual corresponding actions may start sooner or later, and may end sooner or later
 *   - in addition, may need to constantly break up scopes to use elsewhere
 *   - but as long as scopes, inner scopes, etc., are being created and reused, should be good enough to make progress
 * 
 * - specifically, here, the states created are too loose
 *   - i.e., they likely include more actions than is relevant for the state
 *     - so from the outside, there's more that's abstracted, but from the inside, will miss possible sub-scopes
 *       - though sub-scopes will likely be created on branch
 *         - so again, probably good enough
 */

#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_states;
	std::vector<FamilyDefinition*> state_families;
	std::vector<ClassDefinition*> default_state_classes;

	// loop stuff

	std::vector<AbstractNode*> nodes;

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void halfway_activate(std::vector<int>& starting_node_ids,
						  std::vector<std::vector<double>*>& starting_state_vals,	// use pointers so sequence keeps values
						  std::vector<std::vector<bool>>& starting_states_initialized,
						  std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  ScopeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void halfway_backprop(std::vector<std::vector<double>*>& starting_state_errors,
						  std::vector<BackwardContextLayer>& context,
						  double& scale_factor_error,
						  RunHelper& run_helper,
						  ScopeHistory* history);

	void handle_node_activate_helper(int iter_index,
									 int& curr_node_id,
									 std::vector<double>& flat_vals,
									 std::vector<ForwardContextLayer>& context,
									 int& exit_depth,
									 int& exit_node_id,
									 RunHelper& run_helper,
									 ScopeHistory* history);
	void handle_node_backprop_helper(int iter_index,
									 int h_index,
									 std::vector<BackwardContextLayer>& context,
									 double& scale_factor_error,
									 RunHelper& run_helper,
									 ScopeHistory* history);

};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	bool exceeded_depth;
	
};

#endif /* SCOPE_H */