#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class Layer;

class ScoreNetwork {
public:
	/**
	 * - index 0 is global
	 */
	std::vector<int> context_indexes;
	std::vector<int> state_indexes;
	Layer* state_input;

	Layer* output;

	int epoch_iter;
	double output_average_max_update;

	ScoreNetwork(std::vector<int> context_indexes,
				 std::vector<int> state_indexes);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(std::vector<std::vector<double>*>& state_vals);
	void activate(std::vector<std::vector<double>*>& state_vals,
				  ScoreNetworkHistory* history);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		ScoreNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<std::vector<double>*>& state_errors);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<std::vector<double>*>& state_errors,
		ScoreNetworkHistory* history);

	void add_state(int context_index,
				   int state_index);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ScoreNetworkHistory {
public:
	ScoreNetwork* network;

	std::vector<double> state_input_history;

	ScoreNetworkHistory(ScoreNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* SCORE_NETWORK_H */