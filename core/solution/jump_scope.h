#ifndef JUMP_SCOPE_H
#define JUMP_SCOPE_H

#include <vector>

#include "solution_node.h"

const int JUMP_SCOPE_STATE_ENTER = 0;
const int JUMP_SCOPE_STATE_IF = 1;
const int JUMP_SCOPE_STATE_EXIT = 2;

const int JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND = 0;
const int JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START = 1;

class JumpScope : public SolutionNode {
public:
	int num_states;

	std::vector<SolutionNode*> top_path;

	std::vector<std::vector<SolutionNode*>> child_paths;
	std::vector<Network*> child_score_networks;

	// double if_explore_weight;
	// int if_explore_type;
	// int if_explore_state;
	// std::vector<SolutionNode*> if_explore_potential_nodes;
	// Network* if_explore_score_network;	// train greedily
	// Network* if_explore_full_score_network;
	// std::vector<FlatNetwork*> if_explore_state_networks;

	JumpScope(std::vector<int> local_state_sizes,
			  int num_states);
	JumpScope(std::vector<int>& scope_states,
			  std::vector<int>& scope_locations,
			  std::ifstream& save_file);
	~JumpScope();

	SolutionNode* re_eval(Problem& problem,
						  double& predicted_score,
						  std::vector<std::vector<double>>& state_vals,
						  std::vector<SolutionNode*>& scopes,
						  std::vector<int>& scope_states,
						  std::vector<ReEvalStepHistory>& instance_history,
						  std::vector<AbstractNetworkHistory*>& network_historys) override;
	void re_eval_backprop(double score,
						  std::vector<std::vector<double>>& state_errors,
						  std::vector<ReEvalStepHistory>& instance_history,
						  std::vector<AbstractNetworkHistory*>& network_historys) override;
	SolutionNode* explore(Problem& problem,
						  double& predicted_score,
						  std::vector<std::vector<double>>& state_vals,
						  std::vector<SolutionNode*>& scopes,
						  std::vector<int>& scope_states,
						  std::vector<int>& scope_locations,
						  IterExplore*& iter_explore,
						  std::vector<ExploreStepHistory>& instance_history,
						  std::vector<AbstractNetworkHistory*>& network_historys,
						  bool& abandon_instance) override;
	void explore_backprop(double score,
						  std::vector<std::vector<double>>& state_errors,
						  IterExplore*& iter_explore,
						  std::vector<ExploreStepHistory>& instance_history,
						  std::vector<AbstractNetworkHistory*>& network_historys) override;
	void explore_increment(double score,
						   IterExplore*& iter_explore) override;

	void re_eval_increment() override;

	SolutionNode* deep_copy(int inclusive_start_layer) override;
	void set_is_temp_node(bool is_temp_node) override;
	void initialize_local_state(std::vector<int>& explore_node_local_state_sizes) override;
	void setup_flat(std::vector<int>& loop_scope_counts,
					int& curr_index,
					SolutionNode* explore_node) override;
	void setup_new_state(SolutionNode* explore_node,
						 int new_state_size) override;
	void get_min_misguess(double& min_misguess) override;
	void cleanup_explore(SolutionNode* explore_node) override;
	void collect_new_state_networks(SolutionNode* explore_node,
									std::vector<SolutionNode*>& existing_nodes,
									std::vector<Network*>& new_state_networks) override;
	void insert_scope(int layer,
					  int new_state_size) override;
	void reset_explore() override;

	void save(std::vector<int>& scope_states,
			  std::vector<int>& scope_locations,
			  std::ofstream& save_file) override;
	void save_for_display(std::ofstream& save_file) override;

	void activate_child_networks(Problem& problem,
								 std::vector<double>& layer_state_vals,
								 int& best_index,
								 double& best_score);
	void activate_child_networks(Problem& problem,
								 std::vector<double>& layer_state_vals,
								 int& best_index,
								 double& best_score,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_child_networks(double score,
								 std::vector<double>& layer_state_errors,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_child_networks_errors_with_no_weight_change(
		double score,
		std::vector<double>& layer_state_errors,
		std::vector<AbstractNetworkHistory*>& network_historys);
};

#endif /* JUMP_SCOPE_H */