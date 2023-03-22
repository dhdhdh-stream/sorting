#ifndef SCOPE_H
#define SCOPE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "state_network.h"

class FoldHistory;
class ScopeHistory;
class Scope {
public:
	int id;

	int num_local_states;
	int num_input_states;	// also becomes output

	bool is_loop;
	std::vector<bool> starting_state_network_target_is_local;
	std::vector<int> starting_state_network_target_indexes;
	std::vector<StateNetwork*> starting_state_networks;
	StateNetwork* continue_score_network;
	StateNetwork* continue_misguess_network;
	StateNetwork* halt_score_network;
	StateNetwork* halt_misguess_network;
	// Note: eventually, ideally, right choice will have score 0.0 while other will be negative

	std::vector<AbstractNode*> nodes;

	Scope(int num_local_states,
		  int num_input_states,
		  bool is_loop,
		  StateNetwork* continue_score_network,
		  StateNetwork* continue_misguess_network,
		  StateNetwork* halt_score_network,
		  StateNetwork* halt_misguess_network,
		  std::vector<AbstractNode*> nodes);
	Scope(std::ifstream& input_file);
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

	void save(std::ofstream& output_file);

	void new_outer_to_local(int new_outer_size);
	void new_outer_to_input(int new_outer_size);

	void backprop_explore_fold_helper(std::vector<double>& local_state_errors,
									  std::vector<double>& input_errors,
									  double target_val,
									  double final_misguess,
									  double& predicted_score,
									  double& scale_factor,
									  RunHelper& run_helper,
									  ScopeHistory* history);
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