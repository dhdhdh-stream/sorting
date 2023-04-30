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

	void state_hidden_add_new_inner();
	void state_hidden_add_new_outer();
	void state_hidden_remove_new_outer();
	void state_hidden_zero_new_input(int index);
	void state_hidden_zero_state(int index);
	void state_hidden_zero_new_outer(int index);
	void state_hidden_update_state_size(int state_size_increase);
	void state_hidden_new_external_weights_to_state();
	void state_hidden_new_sequence_finalize();
	void state_hidden_remove_state(int index);
	void state_hidden_add_state(int size);
};

#endif /* LAYER_H */