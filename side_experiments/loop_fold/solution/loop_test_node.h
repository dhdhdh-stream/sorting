#ifndef LOOP_TEST_NODE_H
#define LOOP_TEST_NODE_H

#include <vector>

#include "fold_combine_network.h"
#include "fold_loop_init_network.h"
#include "fold_loop_network.h"
#include "compression_network.h"
#include "network.h"
#include "node.h"
#include "score_network.h"
#include "state_network.h"

class LoopTestNode {
public:
	int obs_size;

	int state;
	int state_iter;
	double sum_error;
	double best_sum_error;	// for tune
	double tune_try;

	std::vector<int> outer_scope_sizes;
	Network* init_network;
	FoldCombineNetwork* combine_network;

	std::vector<int> curr_inner_scope_sizes;
	FoldLoopNetwork* curr_loop;

	std::vector<int> test_inner_scope_sizes;
	FoldLoopNetwork* test_loop;

	ScoreNetwork* score_network;

	StateNetwork* state_network;

	std::vector<int> compress_num_scopes;
	std::vector<int> compress_sizes;
	std::vector<CompressionNetwork*> compression_networks;
	std::vector<std::vector<int>> compressed_scope_sizes;

	int test_compress_num_scopes;
	int test_compress_sizes;
	CompressionNetwork* test_compression_network;
	std::vector<int> test_compressed_scope_sizes;

	bool just_score;
	bool update_existing_scope;

	int new_scope_size;

	LoopTestNode(std::vector<int> outer_scope_sizes,
				 Network* init_network,
				 std::vector<int> initial_inner_scope_sizes,
				 FoldLoopNetwork* original_loop,
				 FoldCombineNetwork* combine_network,
				 int obs_size);
	~LoopTestNode();

	void loop_init(std::vector<std::vector<double>>& outer_state_vals,
				   std::vector<double>& loop_state);
	void activate(std::vector<std::vector<double>>& combined_state_vals,
				  std::vector<bool>& combined_scopes_on,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void loop_iter(std::vector<std::vector<double>>& loop_flat_vals,
				   std::vector<double>& loop_state,
				   std::vector<std::vector<double>>& combined_state_vals,
				   std::vector<AbstractNetworkHistory*>& network_historys);
	void process(std::vector<double>& loop_state,
				 std::vector<std::vector<double>>& post_loop_flat_vals,
				 std::vector<std::vector<double>>& combined_state_vals,
				 double target_val,
				 std::vector<Node*>& init_nodes,
				 std::vector<Node*>& loop_nodes,
				 std::vector<AbstractNetworkHistory*>& network_historys);

private:
	void increment();
};

#endif /* LOOP_TEST_NODE_H */