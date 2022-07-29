#ifndef LAYER_H
#define LAYER_H

#include <fstream>
#include <vector>

const int LINEAR_LAYER = 0;
const int RELU_LAYER = 1;

class Layer {
public:
	int type;

	std::vector<double> acti_vals;
	std::vector<double> errors;

	int num_input_layers;
	Layer** input_layers;

	std::vector<std::vector<double>*> weights;
	std::vector<double> constants;
	std::vector<std::vector<double>*> weight_updates;
	std::vector<double> constant_updates;
	std::vector<std::vector<double>*> prev_weight_updates;
	std::vector<double> prev_constant_updates;

	Layer();
	~Layer();
	void setup(int type,
			   int num_input_layers,
			   Layer** input_layers);
	void add_node();
	void add_weights();
	void copy_weights_from(Layer& original);
	void copy_weights_from(std::ifstream& input_file);

	void activate();
	void backprop();
	void calc_max_update(double& max_update_size,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void save_weights(std::ofstream& output_file);
};

#endif /* LAYER_H */