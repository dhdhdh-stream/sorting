#ifndef BUILD_NETWORK_H
#define BUILD_NETWORK_H

#include <fstream>
#include <map>
#include <vector>

class BuildNode;

const int BUILD_MAX_NUM_INPUTS = 10;

class BuildNetwork {
public:
	std::vector<double> inputs;

	std::vector<BuildNode*> nodes;

	std::vector<double> output_weights;
	double output_constant;
	std::vector<double> output_weight_updates;
	double output_constant_update;

	int epoch_iter;
	double average_max_update;

	BuildNetwork();
	~BuildNetwork();

	double activate(std::vector<double>& obs);
	void backprop(std::vector<double>& obs,
				  double target_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(BuildNetwork* original);
};

#endif /* BUILD_NETWORK_H */