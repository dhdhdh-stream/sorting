#ifndef LAYER_H
#define LAYER_H

#include <fstream>
#include <vector>

const int LINEAR_LAYER = 0;
const int RELU_LAYER = 1;
const int LEAKY_LAYER = 2;

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
	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void save_weights(std::ofstream& output_file);

	void backprop_errors_with_no_weight_change();
	void backprop_weights_with_no_error_signal();

	void fold_add_scope(Layer* new_scope_input);
	void fold_pop_scope();
	void fold_backprop_last_state();
	void fold_get_max_update_last_state(double& max_update_size);
	void fold_update_weights_last_state(double learning_rate);
	void fold_backprop_full_state(int state_size);
	void fold_get_max_update_full_state(int state_size,
										double& max_update_size);
	void fold_update_weights_full_state(int state_size,
										double learning_rate);
	void fold_loop_backprop_last_state();
	void fold_loop_backprop_full_state(int state_size);
	void fold_backprop_loop_errors_with_no_weight_change();

	void subfold_add_state(int layer);
	void subfold_backprop_new_state(int layer,
									int new_input_size);
	void subfold_get_max_update_new_state(int layer,
										  int new_input_size,
										  double& max_update_size);
	void subfold_update_weights_new_state(int layer,
										  int new_input_size,
										  double learning_rate);
	void subfold_backprop(int fold_index);
	void subfold_get_max_update(int fold_index,
								double& max_update_size);
	void subfold_update_weights(int fold_index,
								double learning_rate);
};

#endif /* LAYER_H */