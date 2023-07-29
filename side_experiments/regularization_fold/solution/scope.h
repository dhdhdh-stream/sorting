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
	std::vector<bool> state_initialized_locally;
	std::vector<int> state_family_ids;
	std::vector<int> state_default_class_ids;	// if initialized locally

	bool is_loop;
	std::vector<int> starting_target_indexes;
	std::vector<StateNetwork*> starting_state_networks;
	ScoreNetwork* continue_score_network;
	ScoreNetwork* continue_misguess_network;
	ScoreNetwork* halt_score_network;
	ScoreNetwork* halt_misguess_network;
	int furthest_successful_halt;

	std::vector<AbstractNode*> nodes;

	Scope(int id,
		  int num_states,
		  std::vector<bool> state_initialized_locally,
		  std::vector<int> state_family_ids,
		  std::vector<int> state_default_class_ids,
		  bool is_loop,
		  std::vector<int> starting_target_indexes,
		  std::vector<StateNetwork*> starting_state_networks,
		  ScoreNetwork* continue_score_network,
		  ScoreNetwork* continue_misguess_network,
		  ScoreNetwork* halt_score_network,
		  ScoreNetwork* halt_misguess_network);
	Scope(std::ifstream& input_file,
		  int id);
	~Scope();

	void activate(std::vector<int>& starting_node_ids,
				  std::vector<std::vector<double>*>& starting_state_vals,
				  std::vector<std::vector<bool>>& starting_states_initialized,
				  std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void backprop(std::vector<int>& starting_node_ids,
				  std::vector<std::vector<double>*>& starting_state_errors,
				  std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void handle_node_activate_helper(int iter_index,
									 int& curr_node_id,
									 std::vector<double>& flat_vals,
									 std::vector<ForwardContextLayer>& context,
									 bool& experiment_variables_initialized,
									 std::vector<std::vector<StateNetwork*>>*& experiment_scope_state_networks,
									 std::vector<ScoreNetwork*>*& experiment_scope_score_networks,
									 int& experiment_scope_distance,
									 int& exit_depth,
									 int& exit_node_id,
									 RunHelper& run_helper,
									 ScopeHistory* history);
	void experiment_variables_helper(bool& experiment_variables_initialized,
									 std::vector<std::vector<StateNetwork*>>*& experiment_scope_state_networks,
									 std::vector<ScoreNetwork*>*& experiment_scope_score_networks,
									 int& experiment_scope_distance,
									 RunHelper& run_helper);
	void handle_node_backprop_helper(int iter_index,
									 int h_index,
									 std::vector<BackwardContextLayer>& context,
									 double& scale_factor_error,
									 RunHelper& run_helper,
									 ScopeHistory* history);

	void add_state(bool state_initialized,
				   int state_family_id,
				   int state_default_class_id);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<StateNetworkHistory*> starting_state_network_histories;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);	// deep copy for seed
	~ScopeHistory();
};

#endif /* SCOPE_H */