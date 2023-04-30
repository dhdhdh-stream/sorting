#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"
#include "action.h"
#include "fold.h"
#include "loop_fold.h"
#include "run_helper.h"
#include "state_network.h"

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	std::vector<int> state_network_target_indexes;
	std::vector<StateNetwork*> state_networks;

	StateNetwork* score_network;

	int next_node_id;

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;
	double average_sum_impact;

	int explore_curr_try;
	int explore_target_tries;
	double best_explore_surprise;
	std::vector<int> best_explore_scope_context;	// TODO: select randomly with decreasing probability to go outer
	std::vector<int> best_explore_node_context;
	bool best_explore_is_loop;
	int best_explore_exit_depth;
	int best_explore_next_node_id;
	int best_explore_sequence_length;
	std::vector<bool> best_explore_is_inner_scope;
	std::vector<int> best_explore_existing_scope_ids;
	std::vector<Action> best_explore_actions;
	double best_explore_seed_start_predicted_score;
	std::vector<double> best_explore_seed_score_state_vals_snapshot;
	ScopeHistory* best_explore_seed_outer_context_history;
	std::vector<double> best_explore_seed_sequence_state_vals_snapshot;
	std::vector<std::vector<double>> best_explore_seed_sequence_obs_snapshot;
	std::vector<std::vector<ScopeHistory*>> best_explore_seed_inner_context_histories;

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;
	LoopFold* explore_loop_fold;

	ActionNode(std::vector<int> state_network_target_indexes,
			   std::vector<StateNetwork*> state_networks,
			   StateNetwork* score_network);
	ActionNode(std::ifstream& input_file,
			   int scope_id,
			   int scope_index);
	~ActionNode();

	void activate(Problem& problem,
				  std::vector<double>& state_vals,
				  std::vector<bool>& states_initialized,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& states_initialized,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<StateNetworkHistory*> state_network_histories;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	double obs_snapshot;
	std::vector<double> ending_state_snapshot;

	ActionNodeHistory(ActionNode* node,
					  int scope_index);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */