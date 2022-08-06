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

	std::vector<Layer*> input_layers;

	std::vector<std::vector<std::vector<double>>> weights;
	std::vector<double> constants;
	std::vector<std::vector<std::vector<double>>> weight_updates;
	std::vector<double> constant_updates;
	std::vector<std::vector<std::vector<double>>> prev_weight_updates;
	std::vector<double> prev_constant_updates;

	Layer(int type, int num_nodes);
	Layer(Layer* original);
	~Layer();
	
	void setup_weights_full();
	void collapse_input(int input_index,
						Layer* new_collapse_layer);
	void fold_input(Layer* new_fold_layer);	// always folds layer[0] and layer[1]

	void copy_weights_from(Layer* original);

	void activate();
	void backprop();
	void calc_max_update(int layer_index,
						 double& max_update_size,
						 double learning_rate,
						 double momentum);
	void update_weights(int layer_index,
						double factor,
						double learning_rate,
						double momentum);
};

#endif /* LAYER_H */