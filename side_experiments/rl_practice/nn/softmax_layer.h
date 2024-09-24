#ifndef SOFTMAX_LAYER_H
#define SOFTMAX_LAYER_H

#include <fstream>
#include <vector>

class Layer;

class SoftmaxLayer {
public:
	std::vector<double> acti_vals;
	int predicted_index;

	std::vector<Layer*> input_layers;

	std::vector<std::vector<std::vector<double>>> weights;
	std::vector<double> constants;
	std::vector<std::vector<std::vector<double>>> weight_updates;
	std::vector<double> constant_updates;

	SoftmaxLayer(int size);

	void update_structure();

	void load_weights_from(std::ifstream& input_file);

	void activate();
	void activate(int action);
	void backprop(bool is_better);
	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void save_weights(std::ofstream& output_file);
};

#endif /* SOFTMAX_LAYER_H */