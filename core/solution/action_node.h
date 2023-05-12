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

	// don't explore both paths and loops to help prevent some weirdness
	int explore_type;	// TODO: change to setting explore node on empty explore
	// TODO: for loops, literally choose random
	int explore_curr_try;
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
	double best_explore_seed_target_val;

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;
	LoopFold* explore_loop_fold;

	ActionNode(Action action,
			   std::vector<int> state_network_target_indexes,
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
				  double& predicted_score,
				  double& scale_factor,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
	void save_for_display(std::ofstream& output_file);
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
	ActionNodeHistory(ActionNodeHistory* original);	// deep copy for seed
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */