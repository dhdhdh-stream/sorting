#ifndef LAYER_H
#define LAYER_H

#include <fstream>
#include <vector>

#include <Eigen/Dense>

const int LINEAR_LAYER = 0;
const int RELU_LAYER = 1;
const int LEAKY_LAYER = 2;
const int SIGMOID_LAYER = 3;

class Layer {
public:
	int type;

	Eigen::VectorXf acti_vals;
	Eigen::VectorXf errors;

	std::vector<Layer*> input_layers;

	std::vector<std::vector<Eigen::VectorXf>> weights;
	std::vector<double> constants;
	std::vector<std::vector<Eigen::VectorXf>> weight_updates;
	std::vector<double> constant_updates;

	Layer(int type);

	void update_structure();

	void copy_weights_from(Layer* original);
	void load_weights_from(std::ifstream& input_file);

	void activate();
	void backprop();
	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void save_weights(std::ofstream& output_file);
};

#endif /* LAYER_H */