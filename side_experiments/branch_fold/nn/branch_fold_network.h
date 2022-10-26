#ifndef BRANCH_FOLD_NETWORK_H
#define BRANCH_FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class BranchFoldNetwork : public AbstractNetwork {
public:
	std::vector<int> pre_branch_flat_sizes;
	std::vector<int> new_branch_flat_sizes;
	std::vector<int> post_branch_flat_sizes;

	std::vector<Layer*> pre_branch_flat_inputs;
	std::vector<Layer*> new_branch_flat_inputs;
	std::vector<Layer*> post_branch_flat_inputs;

	int fold_index;

	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	BranchFoldNetwork(std::vector<int> pre_branch_flat_sizes,
					  std::vector<int> new_branch_flat_sizes,
					  std::vector<int> post_branch_flat_sizes);
	BranchFoldNetwork(std::ifstream& input_file);
	BranchFoldNetwork(BranchFoldNetwork* original);
	~BranchFoldNetwork();

	void activate(std::vector<std::vector<double>>& pre_branch_flat_vals,
				  std::vector<std::vector<double>>& new_branch_flat_vals,
				  std::vector<std::vector<double>>& post_branch_flat_vals);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* BRANCH_FOLD_NETWORK_H */