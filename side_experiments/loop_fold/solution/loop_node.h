#ifndef LOOP_NODE_H
#define LOOP_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "compression_network.h"
#include "network.h"
#include "score_network.h"
#include "state_network.h"

class LoopNode : public AbstractNode {
public:
	std::string id;

	int loop_state_size;

	Network* init_network;

	Network* halt_score_network;
	Network* halt_misguess_network;
	Network* continue_score_network;
	Network* continue_misguess_network;

	std::vector<AbstractNode*> nodes;
	int inner_num_scopes;
	std::vector<int> inner_scope_sizes;

	Network* loop_network;

	ScoreNetwork* score_network;

	bool just_score;

	bool update_existing_scope;
	int new_scope_size;
	StateNetwork* state_network;

	std::vector<int> compress_num_scopes;
	std::vector<int> compress_sizes;
	std::vector<CompressionNetwork*> compression_networks;

	std::vector<std::vector<int>> compressed_scope_sizes;

	LoopNode(std::string id,
			 Network* init_network,
			 std::vector<AbstractNode*> nodes,
			 std::vector<int> inner_num_scopes,
			 std::vector<int> inner_sizes,
			 Network* loop_network);
	LoopNode(std::string id,
			 std::ifstream& input_file);
	~LoopNode();
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs) override;
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys) override;
	void backprop(double target_val,
				  std::vector<std::vector<double>>& state_errors) override;
	void backprop(double target_val,
				  std::vector<std::vector<double>>& state_errors,
				  std::vector<AbstractNetworkHistory*>& network_historys) override;
	void backprop_zero_train(AbstractNode* original,
							 double& sum_error) override;
	void activate_state(std::vector<std::vector<double>>& state_vals,
						std::vector<bool>& scopes_on,
						std::vector<double>& obs) override;
	void backprop_zero_train_state(AbstractNode* original,
								   double& sum_error) override;

	void get_scope_sizes(std::vector<int>& scope_sizes) override;

	void save(std::ofstream& output_file) override;

	void activate(int num_iters,
				  std::vector<std::vector<std::vector<double>>>& iter_loop_flat_vals,
				  std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs);
	void activate(int num_iters,
				  std::vector<std::vector<std::vector<double>>>& iter_loop_flat_vals,
				  std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);
};

#endif /* LOOP_NODE_H */