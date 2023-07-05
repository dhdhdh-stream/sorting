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
	void lasso_update_weights(double lasso_weight,
							  double learning_rate);
	void lasso_update_weights(std::vector<std::vector<double>>& lasso_weights,
							  double learning_rate);

	void backprop_errors_with_no_weight_change();
	void backprop_weights_with_no_error_signal();

	void save_weights(std::ofstream& output_file);

	void state_hidden_finalize_new_state();
	void state_hidden_finalize_new_input();
	void score_hidden_finalize_new_state(int new_total_states);
	void score_hidden_add_new_state(int new_state_size);
	void exit_hidden_finalize_new_state();
};

#endif /* LAYER_H */