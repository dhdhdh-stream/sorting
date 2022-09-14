#ifndef SOLUTION_NODE_EMPTY_H
#define SOLUTION_NODE_EMPTY_H

#include <map>
#include <vector>

#include "fold_helper.h"
#include "solution_node.h"

class SolutionNodeEmpty : public SolutionNode {
public:
	std::vector<Network*> state_networks;

	std::map<SolutionNode*,FoldHelper*> fold_helpers;

	SolutionNodeEmpty(std::vector<int> local_state_sizes);
	SolutionNodeEmpty(std::vector<int>& scope_states,
					  std::vector<int>& scope_locations,
					  std::ifstream& save_file);
	~SolutionNodeEmpty();

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

	void activate_state_networks(Problem& problem,
								 std::vector<std::vector<double>>& state_vals);
	void activate_state_networks(Problem& problem,
								 std::vector<std::vector<double>>& state_vals,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void activate_state_network(Problem& problem,
								int layer,
								std::vector<double>& layer_state_vals);
	void activate_state_network(Problem& problem,
								int layer,
								std::vector<double>& layer_state_vals,
								std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_state_networks(std::vector<std::vector<double>>& state_errors,
								 std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_state_network_errors_with_no_weight_change(
		int layer,
		std::vector<double>& layer_state_errors,
		std::vector<AbstractNetworkHistory*>& network_historys);

	void new_path_activate_state_networks(double observations,
										  std::vector<std::vector<double>>& state_vals,
										  std::vector<AbstractNetworkHistory*>& network_historys);
};

#endif /* SOLUTION_NODE_EMPTY_H */