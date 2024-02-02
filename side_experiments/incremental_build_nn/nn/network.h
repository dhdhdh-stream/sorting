/**
 * - to supplement linear regression
 *   - so don't initially consider all inputs
 *     - though may build up to consider a lot
 */

#ifndef NETWORK_H
#define NETWORK_H

const int NETWORK_INCREMENT_TYPE_SIDE = 0;
/**
 * - hidden for existing already trained, so hopefully, learning on it will still be fast
 */
const int NETWORK_INCREMENT_TYPE_ABOVE = 1;

class Network {
public:
	/**
	 * - track input/input context in nodes
	 */
	std::vector<Layer*> inputs;

	std::vector<Layer*> hiddens;
	/**
	 * - (is_input, index)
	 */
	std::vector<std::vector<std::pair<bool,int>>> hidden_inputs;

	Layer* output;
	std::vector<std::pair<bool,int>> output_inputs;

	int epoch_iter;
	std::vector<double> hidden_average_max_updates;
	double output_average_max_update;

	Network(int input_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<std::vector<double>>& input_vals);
	void backprop(double error);

	void increment_side(int input_size);
	void increment_above(int input_size);
	void finalize();

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */