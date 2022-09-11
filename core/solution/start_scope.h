#ifndef START_SCOPE_H
#define START_SCOPE_H

#include <vector>

#include "solution_node.h"

const int START_SCOPE_STATE_ENTER = 0;
const int START_SCOPE_STATE_EXIT = 1;

class StartScope : public SolutionNode {
public:
	std::vector<SolutionNode*> path;

	int jump_end_non_inclusive_index;

	StartScope();
	StartScope(std::vector<int>& scope_states,
			   std::vector<int>& scope_locations,
			   std::ifstream& save_file);
	~StartScope();

	SolutionNode* re_eval(Problem& problem,
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
};

#endif /* START_SCOPE_H */