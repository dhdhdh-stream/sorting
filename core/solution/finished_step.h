#ifndef FINISHED_STEP_H
#define FINISHED_STEP_H

class FinishedStep {
public:
	int scope_type;	// only SCOPE_TYPE_ACTION or SCOPE_TYPE_SCOPE
	AbstractScope* scope;

	FoldNetwork* inner_input_network;	// adds to s_input

	int new_layer_size;
	Network* obs_network;

	SubFoldNetwork* score_network;

	int compress_num_layers;	// always >1
	int compress_size;
	SubFoldNetwork* compress_network;
	std::vector<int> compressed_scope_sizes;	// earliest to latest
	std::vector<int> compressed_s_input_sizes;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<SmallNetwork*> input_networks;

	FinishedStep(AbstractScope* scope,
				 SubFoldNetwork* inner_input_network,
				 int new_layer_size,
				 Network* obs_network,
				 SubFoldNetwork* score_network,
				 int compress_num_layers,
				 int compress_new_size,
				 CompressNetwork* compress_network,
				 std::vector<int> compressed_scope_sizes,
				 std::vector<int> compressed_s_input_sizes,
				 std::vector<int> input_layer,
				 std::vector<int> input_sizes,
				 std::vector<SmallNetwork*> input_networks);
	~FinishedStep();
	void activate(Problem& problem,
				  std::vector<std::vector<double>>& state_vals,
				  std::vector<std::vector<double>>& s_input_vals,
				  double& predicted_score);
};

#endif /* FINISHED_STEP_H */