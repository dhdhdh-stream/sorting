#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "loop_fold.h"
#include "run_helper.h"
#include "scale.h"
#include "scope.h"
#include "state_network.h"

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	std::vector<int> pre_state_network_target_indexes;
	std::vector<StateNetwork*> pre_state_networks;

	int inner_scope_id;
	std::vector<int> inner_input_indexes;
	std::vector<int> inner_input_target_indexes;
	Scale* scope_scale_mod;

	std::vector<int> post_state_network_target_indexes;
	std::vector<StateNetwork*> post_state_networks;

	StateNetwork* score_network;

	int next_node_id;
	// may go elsewhere due to early exit, but handled from inner

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;
	double average_sum_impact;

	int explore_curr_try;
	int explore_target_tries;
	double best_explore_surprise;
	std::vector<int> best_explore_scope_context;
	std::vector<int> best_explore_node_context;
	bool best_explore_is_loop;
	int best_explore_exit_depth;
	int best_explore_next_node_id;
	std::vector<bool> best_explore_is_inner_scope;
	std::vector<int> best_explore_existing_scope_ids;
	std::vector<Action> best_explore_actions;
	double best_explore_seed_start_predicted_score;
	double best_explore_seed_start_scale_factor;
	std::vector<double> best_explore_seed_state_vals_snapshot;
	ScopeHistory* best_explore_seed_outer_context_history;	// deep copy before continuing past explore node

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;
	LoopFold* explore_loop_fold;

	ScopeNode(std::vector<int> pre_state_network_target_indexes,
			  std::vector<StateNetwork*> pre_state_networks,
			  int inner_scope_id,
			  std::vector<int> inner_input_indexes,
			  std::vector<int> inner_input_target_indexes,
			  Scale* scope_scale_mod,
			  std::vector<int> post_state_network_target_indexes,
			  std::vector<StateNetwork*> post_state_networks,
			  StateNetwork* score_network);
	ScopeNode(std::ifstream& input_file,
			  int scope_id,
			  int scope_index);
	~ScopeNode();

	void activate(Problem& problem,
				  std::vector<double>& state_vals,
				  std::vector<bool>& states_initialized,
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
				  ScopeNodeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& states_initialized,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	std::vector<StateNetworkHistory*> pre_state_network_histories;

	ScopeHistory* inner_scope_history;

	bool inner_is_early_exit;
	std::vector<StateNetworkHistory*> post_state_network_histories;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	ScopeNodeHistory(ScopeNode* node,
					 int scope_index);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */