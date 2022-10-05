#ifndef LOOP_INIT_TEST_NODE_H
#define LOOP_INIT_TEST_NODE_H

#include <vector>

#include "fold_combine_network.h"
#include "fold_loop_init_network.h"
#include "fold_loop_network.h"
#include "compression_network.h"
#include "node.h"
#include "score_network.h"
#include "state_network.h"

class LoopInitTestNode {
public:
	int obs_size;

	int state;
	int state_iter;
	double sum_error;
	double best_sum_error;	// for tune
	double tune_try;

	std::vector<int> curr_outer_scope_sizes;
	FoldLoopInitNetwork* curr_init;
	FoldLoopNetwork* curr_loop;
	FoldCombineNetwork* curr_combine;

	std::vector<int> test_outer_scope_sizes;
	FoldLoopInitNetwork* test_init;
	FoldLoopNetwork* test_loop;
	FoldCombineNetwork* test_combine;

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

	LoopInitTestNode(std::vector<int> initial_outer_scope_sizes,
					 FoldLoopInitNetwork* original_init,
					 FoldLoopNetwork* original_loop,
					 FoldCombineNetwork* original_combine,
					 int obs_size);
	~LoopInitTestNode();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs);
	void loop_init(std::vector<std::vector<double>>& pre_loop_flat_vals,
				   std::vector<std::vector<double>>& state_vals,
				   std::vector<double>& loop_state);
	void loop_iter(std::vector<std::vector<double>>& pre_loop_flat_vals,
				   std::vector<std::vector<double>>& loop_flat_vals,
				   std::vector<double>& loop_state,
				   std::vector<std::vector<double>>& state_vals,
				   std::vector<AbstractNetworkHistory*>& network_historys);
	void process(std::vector<double>& loop_state,
				 std::vector<std::vector<double>>& pre_loop_flat_vals,
				 std::vector<std::vector<double>>& post_loop_flat_vals,
				 std::vector<std::vector<double>>& state_vals,
				 double target_val,
				 std::vector<Node*>& init_nodes,
				 std::vector<AbstractNetworkHistory*>& network_historys);

private:
	void increment();
};

#endif /* LOOP_INIT_TEST_NODE_H */