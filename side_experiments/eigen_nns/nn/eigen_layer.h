#ifndef EIGEN_LAYER_H
#define EIGEN_LAYER_H

#include <fstream>
#include <vector>

#include <Eigen/Dense>

const int LINEAR_LAYER = 0;
const int RELU_LAYER = 1;
const int LEAKY_LAYER = 2;
const int SIGMOID_LAYER = 3;

class EigenLayer {
public:
	int type;

	Eigen::VectorXf acti_vals;
	Eigen::VectorXf errors;

	std::vector<EigenLayer*> input_layers;

	std::vector<std::vector<Eigen::VectorXf>> weights;
	std::vector<float> constants;
	std::vector<std::vector<Eigen::VectorXf>> weight_updates;
	std::vector<float> constant_updates;

	EigenLayer(int type);

	void update_structure();

	void copy_weights_from(EigenLayer* original);
	void load_weights_from(std::ifstream& input_file);

	void activate();
	void backprop();
	void backprop_through();
	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void save_weights(std::ofstream& output_file);
};

#endif /* EIGEN_LAYER_H */