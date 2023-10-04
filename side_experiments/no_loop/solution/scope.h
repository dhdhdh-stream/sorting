/**
 * TODO: to handle loops, average values, and possibly select one iter/context to focus on
 * - even if only one iter is relevant, and that iter changes, will still be correlation
 */

#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_input_states;
	int num_local_states;

	/**
	 * - no need to explicitly track score states here
	 */

	std::vector<AbstractNode*> nodes;

	double average_score;
	double score_variance;
	/**
	 * - measure using sqr over abs
	 *   - even though sqr may not measure true score improvement, it measures information improvement
	 *     - which ultimately leads to better branching
	 */
	double average_misguess;
	double misguess_variance;

	std::vector<Scope*> child_scopes;
	/**
	 * - for constructing new sequences
	 */

	ObsExperiment* obs_experiment;

	Scope();
	~Scope();

	void activate(std::vector<int>& starting_node_ids,
				  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
				  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
				  std::vector<double>& flat_vals,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void node_activate_helper(int iter_index,
							  int& curr_node_id,
							  std::vector<double>& flat_vals,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  int& exit_node_id,
							  RunHelper& run_helper,
							  ScopeHistory* history);

	void random_activate(std::vector<int>& starting_node_ids,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& exit_depth,
						 int& exit_node_id,
						 int& num_nodes,
						 ScopeHistory* history);
	void node_random_activate_helper(int& curr_node_id,
									 std::vector<int>& scope_context,
									 std::vector<int>& node_context,
									 int& exit_depth,
									 int& exit_node_id,
									 int& num_nodes,
									 ScopeHistory* history);

	void create_sequence_activate(std::vector<int>& starting_node_ids,
								  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
								  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
								  std::vector<std::map<std::pair<bool,int>, int>>& starting_state_mappings,
								  std::vector<double>& flat_vals,
								  std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes,
								  RunHelper& run_helper);
	void node_create_sequence_activate_helper(int& curr_node_id,
											  std::vector<double>& flat_vals,
											  std::vector<ContextLayer>& context,
											  int target_num_nodes,
											  int& curr_num_nodes,
											  Sequence* new_sequence,
											  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
											  int& new_num_input_states,
											  std::vector<AbstractNode*>& new_nodes,
											  RunHelper& run_helper);

	void update_backprop(double target_val,
						 ScopeHistory* history);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  int id);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	std::map<State*, StateStatus> score_state_snapshots;

	// for ObsExperiment
	std::vector<int> test_obs_indexes;
	std::vector<double> test_obs_vals;

	BranchExperimentHistory* inner_branch_experiment_history;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */