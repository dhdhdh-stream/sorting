#ifndef SCOPE_H
#define SCOPE_H

#include "fold.h"
#include "network.h"

class Scope {
public:
	int id;

	// if input expanded somewhere else, initially 0.0
	// these uninitialized inputs are like a call for information
	// can search for outer input that matches
	// so can add input context by context
	// don't have to add all at once

	// TODO: make explore input context based as well

	// within that context, can support loops, branches, etc. but has to be single ancestor

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

	void explore_activate(std::vector<double>& input_vals,
						  std::vector<double>& local_state_vals,
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

	void existing_explore_activate(std::vector<double>& input_vals,
								   std::vector<std::vector<double>>& flat_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history);
};

class ScopeHistory {
public:
	std::vector<AbstractNodeHistory*> node_histories;

	int explore_index;	// based off node_histories
};

#endif /* SCOPE_H */