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

	Layer(int type, int num_nodes);
	Layer(Layer* original);
	~Layer();
	
	void setup_weights_full();

	void copy_weights_from(Layer* original);
	void load_weights_from(std::ifstream& input_file);

	void activate();
	void backprop();
	void calc_max_update(double& max_update_size,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void save_weights(std::ofstream& output_file);

	void backprop_errors_with_no_weight_change();

	void fold_setup_next();
	void activate_as_linear();
	void fold_backprop_next();
	void fold_calc_max_update_next(double& max_update_size,
								   double learning_rate);
	void fold_update_weights_next(double factor,
								  double learning_rate);
	void fold_take_next();
	void fold_discard_next();
};

#endif /* LAYER_H */