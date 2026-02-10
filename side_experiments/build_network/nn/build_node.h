#ifndef BUILD_NODE_H
#define BUILD_NODE_H

#include <fstream>
#include <vector>

class BuildNetwork;
class Layer;

const int INPUT_TYPE_INPUT = 0;
const int INPUT_TYPE_NODE = 1;

class BuildNode {
public:
	std::vector<int> input_types;
	std::vector<int> input_indexes;

	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	BuildNode(std::vector<int>& input_types,
			  std::vector<int>& input_indexes);
	BuildNode(std::ifstream& input_file);
	~BuildNode();

	void init_activate(std::vector<double>& input_vals,
					   std::vector<double>& node_vals);
	void init_backprop();

	void activate(BuildNetwork* network);
	void backprop(BuildNetwork* network);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void save(std::ofstream& output_file);
};

#endif /* BUILD_NODE_H */