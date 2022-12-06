#ifndef FINISHED_STEP_H
#define FINISHED_STEP_H

#include <vector>

#include "fold_network.h"
#include "scope.h"

class FinishedStep {
public:
	bool is_inner_scope;
	Scope* scope;
	int obs_size;

	std::vector<int> inner_input_input_layer;
	std::vector<int> inner_input_input_sizes;
	std::vector<FoldNetwork*> inner_input_input_networks;
	FoldNetwork* inner_input_network;
	double scope_scale_mod;

	FoldNetwork* score_network;

	int compress_num_layers;
	bool active_compress;
	int compress_new_size;
	FoldNetwork* compress_network;
	int compress_original_size;	// for constructing scope

	std:;vector<int> compressed_s_input_sizes;	// earliest to latest
	std::vector<int> compressed_scope_sizes;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<FoldNetwork*> input_networks;


};

class FinishedStepHistory {
public:
	FinishedStep* finished_step;

	std::vector<FoldNetworkHistory*> inner_input_input_network_histories;
	FoldNetworkHistory* inner_input_network_history;

	ScopeHistory* scope_history;

	FoldNetworkHistory* score_network_history;
	double score_update;

	FoldNetworkHistory* compress_network_history;

	std::vector<FoldNetworkHistory*> input_network_histories;
};

#endif /* FINISHED_STEP_H */