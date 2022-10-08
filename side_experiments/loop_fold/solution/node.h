#ifndef NODE_H
#define NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "compression_network.h"
#include "score_network.h"
#include "state_network.h"

class Node : public AbstractNode {
public:
	std::string id;

	ScoreNetwork* score_network;

	bool just_score;

	bool update_existing_scope;
	int new_scope_size;
	StateNetwork* state_network;

	std::vector<int> compress_num_scopes;
	std::vector<int> compress_sizes;
	std::vector<CompressionNetwork*> compression_networks;

	std::vector<std::vector<int>> compressed_scope_sizes;

	Node(std::string id,
		 ScoreNetwork* score_network,
		 bool just_score,
		 bool update_existing_scope,
		 int new_scope_size,
		 StateNetwork* state_network,
		 std::vector<int> compress_num_scopes,
		 std::vector<int> compress_sizes,
		 std::vector<CompressionNetwork*> compression_networks,
		 std::vector<std::vector<int>> compressed_scope_sizes);
	Node(std::string id,
		 std::ifstream& input_file);
	Node(Node* original);
	~Node();
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs) override;
	// only on tune
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
};

#endif /* NODE_H */