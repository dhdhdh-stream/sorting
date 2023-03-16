#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "run_helper.h"
#include "state_network.h"

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	std::vector<bool> state_network_target_is_local;
	std::vector<int> state_network_target_indexes;
	std::vector<StateNetwork*> state_networks;

	StateNetwork* score_network;

	int next_node_id;	// TODO: set when adding to Scope?

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;
	double average_sum_impact;

	// TODO: add batch surprise and seeding

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	// TODO: explore_action_sequences
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;

	ActionNode(std::vector<bool> state_network_target_is_local,
			   std::vector<int> state_network_target_indexes,
			   std::vector<StateNetwork*> state_networks,
			   StateNetwork* score_network);
	ActionNode(std::ifstream& input_file,
			   int scope_id,
			   int scope_index);
	~ActionNode();

	void activate(std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
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
	std::vector<double> ending_local_state_snapshot;
	std::vector<double> ending_input_state_snapshot;

	ActionNodeHistory(ActionNode* node,
					  int scope_index);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */