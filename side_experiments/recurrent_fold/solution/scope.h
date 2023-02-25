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

	int num_input_states;	// also becomes output
	int num_local_states;

	bool is_loop;
	Network* continue_network;
	Network* halt_network;

	std::vector<AbstractNode*> nodes;
	// TODO: for now, on explore replace, update connection, but leave original nodes as is, but think about garbage collection

	// if trying entire sequences, then don't need to worry about exploring at start of scope

	Scope(int num_input_states,
		  int num_local_states,
		  bool is_loop,
		  Network* continue_network,
		  Network* halt_network,
		  std::vector<AbstractNode*> nodes);
	~Scope();

	void activate(std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  int& early_exit_count,
				  Fold*& early_exit_fold,
				  int& early_exit_index);

	// TODO: leave explore for scope activate so implementation is cleaner
};

#endif /* SCOPE_H */