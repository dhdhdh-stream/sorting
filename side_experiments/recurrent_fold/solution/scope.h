#ifndef SCOPE_H
#define SCOPE_H

#include "fold.h"
#include "network.h"

class Scope {
public:
	int id;

	int num_local_states;
	int num_input_states;	// also becomes output

	bool is_loop;
	Network* continue_network;
	Network* halt_network;

	std::vector<AbstractNode*> nodes;
	// TODO: for now, on explore replace, update connection, but leave original nodes as is, but think about garbage collection

	// if trying entire sequences, then don't need to worry about exploring at start of scope

	ExploreWeight* starting_explore_weight;

	// TODO: explore weight has to be proportional, rather than based directly on size of score?
	// branches change explore weight?
	// add explore weight scale factor

	Scope(int num_input_states,
		  int num_local_states,
		  bool is_loop,
		  Network* continue_network,
		  Network* halt_network,
		  std::vector<AbstractNode*> nodes);
	~Scope();

	void explore_on_path_activate(std::vector<double>& input_vals,
								  std::vector<std::vector<double>>& flat_vals,
								  double& predicted_score,
								  double& scale_factor,
								  std::vector<int>& scope_context,
								  std::vector<int>& node_context,
								  std::vector<int>& context_iter,
								  std::vector<ContextHistory*>& context_histories,
								  int& early_exit_depth,
								  int& early_exit_node_id,
								  FoldHistory*& early_exit_fold_history,
								  int& explore_exit_depth,
								  int& explore_exit_node_id,
								  FoldHistory*& explore_exit_fold_history,
								  RunHelper& run_helper,
								  ScopeHistory* history);
	void explore_on_path_backprop(std::vector<double>& input_errors,
								  double target_val,
								  double final_misguess,
								  double& predicted_score,
								  double& scale_factor,
								  RunHelper& run_helper,
								  ScopeHistory* history);

	void explore_off_path_activate();
	void explore_off_path_backprop();

	void existing_explore_activate();
	void existing_explore_backprop();

	void update_activate(std::vector<double>& input_vals,
						 std::vector<std::vector<double>>& flat_vals,
						 double& predicted_score,
						 double& scale_factor,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 std::vector<int>& context_iter,
						 std::vector<ContextHistory*>& context_history,
						 int& early_exit_depth,
						 int& early_exit_node_id,
						 FoldHistory*& early_exit_fold_history,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void update_backprop(double target_val,
						 double final_misguess,
						 double& predicted_score,
						 double& scale_factor,
						 ScopeHistory* history);

	void existing_update_activate();
	void existing_update_backprop();
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	bool is_explore_sequence;

	double sum_impact;
	double ending_explore_weight_scale_factor;
};

#endif /* SCOPE_H */