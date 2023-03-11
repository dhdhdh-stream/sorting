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

	Scope(int num_local_states,
		  int num_input_states,
		  bool is_loop,
		  Network* continue_network,
		  Network* halt_network,
		  std::vector<AbstractNode*> nodes);
	~Scope();

	void activate(std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<ScopeHistory*>& context_histories,
				  int& early_exit_depth,
				  int& early_exit_node_id,
				  FoldHistory*& early_exit_fold_history,
				  int& explore_exit_depth,
				  int& explore_exit_node_id,
				  FoldHistory*& explore_exit_fold_history,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void backprop(std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void new_outer_to_local(int new_outer_size);
	void new_outer_to_input(int new_outer_size);

};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	int explore_index;
	FoldHistory* explore_fold_history;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */