#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <vector>

#include "abstract_network.h"

class AbstractNode {
public:
	virtual ~AbstractNode() {};

	virtual void activate(std::vector<std::vector<double>>& state_vals,
						  std::vector<bool>& scopes_on,
						  std::vector<double>& obs) = 0;
	virtual void activate(std::vector<std::vector<double>>& state_vals,
						  std::vector<bool>& scopes_on,
						  std::vector<double>& obs,
						  std::vector<AbstractNetworkHistory*>& network_historys) = 0;
	virtual void backprop(double target_val,
						  std::vector<std::vector<double>>& state_errors) = 0;
	virtual void backprop(double target_vals,
						  std::vector<std::vector<double>>& state_errors,
						  std::vector<AbstractNetworkHistory*>& network_historys) = 0;
	virtual void backprop_zero_train(AbstractNode* original,
									 double& sum_error) = 0;
	virtual void activate_state(std::vector<std::vector<double>>& state_vals,
								std::vector<bool>& scopes_on,
								std::vector<double>& obs) = 0;
	virtual void backprop_zero_train_state(AbstractNode* original,
										   double& sum_error) = 0;

	virtual void get_scope_sizes(std::vector<int>& scope_sizes) = 0;

	virtual void save(std::ofstream& output_file) = 0;
};

#endif /* ABSTRACT_NODE_H */