#ifndef SCOPE_H
#define SCOPE_H

#include "fold.h"
#include "network.h"
#include "scope_path.h"

class ScopePath;
class Scope {
public:
	int num_input_states;	// also becomes output
	int num_local_states;	// for loops

	bool is_loop;
	Network* continue_network;	// i.e., also combined_score_network -- non-NULL if is_loop or more than 1 path
	Network* halt_network;

	std::vector<Network*> score_networks;	// NULL if only 1 path
	// Note: eventually, ideally, right branch will have score 0.0 while others will be negative
	std::vector<bool> is_fold;
	std::vector<ScopePath*> branches;
	std::vector<Fold*> folds;
	std::vector<int> num_travelled;	// if there's recursion, scores may be inaccurate, so ease in to new branch
	// TODO: track when last travelled, and delete unused

	Scope(int num_input_states,
		  int num_local_states,
		  bool is_loop,
		  Network* continue_network,
		  Network* halt_network,
		  std::vector<Network*> score_networks,
		  std::vector<bool> is_fold,
		  std::vector<ScopePath*> branches,
		  std::vector<Fold*> folds,
		  std::vector<int> num_travelled);
	~Scope();

	void activate(std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score);
};

#endif /* SCOPE_H */