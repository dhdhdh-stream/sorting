#ifndef BUILD_NODE_H
#define BUILD_NODE_H

#include <fstream>
#include <vector>

class BuildNetwork;
class Layer;

class BuildNode {
public:
	std::vector<int> input_indexes;
	/**
	 * TODO:
	 * - normalize inputs
	 *   - base on init snapshot
	 *     - so different nodes will have different averages/standard deviations
	 */

	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	BuildNode(std::vector<int>& input_indexes);
	BuildNode(BuildNode* original);
	BuildNode(std::ifstream& input_file);
	~BuildNode();

	void activate(std::vector<double>& input_vals);
	void backprop();

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void save(std::ofstream& output_file);
};

#endif /* BUILD_NODE_H */