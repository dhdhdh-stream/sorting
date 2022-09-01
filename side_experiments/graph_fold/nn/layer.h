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

	bool is_on;

	Layer(int type, int num_nodes);
	Layer(Layer* original);
	~Layer();
	
	void setup_weights_full();

	void copy_weights_from(Layer* original);
	void load_weights_from(std::ifstream& input_file);

	void activate();
	void backprop();
	void calc_max_update(double& max_update_size,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void save_weights(std::ofstream& output_file);

	void backprop_errors_with_no_weight_change();

	void add_potential_input_layer(Layer* potential);
	void input_extend();
	void hidden_extend_with_potential(int potential_index,
									  Layer* potential_hidden);
	void output_extend_with_potential(int potential_index);
	void delete_potential_input_layer(int potential_index);
	void hidden_input_extend();
	void input_trim();
	void hidden_trim(int index);
	void remove_potential_input_layers();

	void backprop_fold_state();
	void calc_max_update_fold_state(double& max_update_size,
									double learning_rate,
									double momentum);
	void update_weights_fold_state(double factor,
								   double learning_rate,
								   double momentum);
};

#endif /* LAYER_H */