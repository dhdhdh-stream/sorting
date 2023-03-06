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
								  std::vector<ContextHistory*>& context_history,
								  int& exit_depth,
								  Fold*& explore_fold,
								  int& exit_index,
								  FoldHistory*& explore_fold_history,
								  RunHelper& run_helper);
	void explore_on_path_backprop();

	void explore_off_path_activate();
	void explore_off_path_backprop();

	void existing_explore_activate(std::vector<double>& input_vals,
								   std::vector<std::vector<double>>& flat_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history);
	void existing_explore_backprop();

	void update_activate();
	void update_backprop();

	void existing_update_activate();
	void existing_update_backprop();
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	bool is_explore_sequence;
};

#endif /* SCOPE_H */