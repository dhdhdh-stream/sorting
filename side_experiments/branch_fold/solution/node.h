#ifndef NODE_H
#define NODE_H

#include <vector>

#include "network.h"
#include "scope.h"
#include "small_network.h"

class Scope;
class Node {
public:
	std::string id;

	bool is_scope;
	std::vector<int> action_input_input_layer;
	std::vector<int> action_input_input_sizes;
	std::vector<SmallNetwork*> action_input_input_networks;
	SmallNetwork* action_input_network;
	// always activate action even if new_layer_size == 0
	Scope* action;

	int obs_size;
	int new_layer_size;
	Network* obs_network;

	std::vector<int> score_input_layer;
	std::vector<int> score_input_sizes;
	std::vector<SmallNetwork*> score_input_networks;
	SmallNetwork* score_network;

	int compress_num_layers;
	int compress_new_size;
	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<SmallNetwork*> input_networks;
	Network* compression_network;
	std::vector<int> compressed_scope_sizes;	// earliest to latest
	std::vector<int> compressed_s_input_sizes;

	Node(std::string id,
		 bool is_scope,
		 std::vector<int> action_input_input_layer,
		 std::vector<int> action_input_input_sizes,
		 std::vector<SmallNetwork*> action_input_input_networks,
		 SmallNetwork* action_input_network,
		 Scope* action,
		 int obs_size,
		 int new_layer_size,
		 Network* obs_network,
		 std::vector<int> score_input_layer,
		 std::vector<int> score_input_sizes,
		 std::vector<SmallNetwork*> score_input_networks,
		 SmallNetwork* score_network,
		 int compress_num_layers,
		 int compress_new_size,
		 std::vector<int> input_layer,
		 std::vector<int> input_sizes,
		 std::vector<SmallNetwork*> input_networks,
		 Network* compression_network,
		 std::vector<int> compressed_scope_sizes,
		 std::vector<int> compressed_s_input_sizes);
	Node(std::ifstream& input_file);
	~Node();
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<std::vector<double>>& s_input_vals,
				  std::vector<double>& obs,
				  double& predicted_score);
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<std::vector<double>>& s_input_vals,
				  std::vector<std::vector<double>>& inner_flat_vals,
				  double& predicted_score);
	void backprop(std::vector<std::vector<double>>& state_errors,
				  std::vector<std::vector<double>>& s_input_errors,
				  double& predicted_score,
				  double target_val);
	void backprop_errors_with_no_weight_change(std::vector<std::vector<double>>& state_errors,
											   std::vector<std::vector<double>>& s_input_errors,
											   double& predicted_score,
											   double target_val);

	void backprop(std::vector<std::vector<double>>& state_errors,
				  std::vector<std::vector<double>>& s_input_errors,
				  double& predicted_score,
				  double target_val,
				  double predicted_score_error);
	void backprop_errors_with_no_weight_change(std::vector<std::vector<double>>& state_errors,
											   std::vector<std::vector<double>>& s_input_errors,
											   double& predicted_score,
											   double target_val,
											   double predicted_score_error);

	void get_scope_sizes(std::vector<int>& scope_sizes,
						 std::vector<int>& s_input_sizes);

	void save(std::ofstream& output_file);
};

#endif /* NODE_H */