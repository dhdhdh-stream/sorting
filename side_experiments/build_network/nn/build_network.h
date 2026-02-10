#ifndef BUILD_NETWORK_H
#define BUILD_NETWORK_H

#include <fstream>
#include <map>
#include <vector>

class BuildNode;

#if defined(MDEBUG) && MDEBUG
const int BUILD_NUM_TRAIN_SAMPLES = 40;
const int BUILD_NUM_TEST_SAMPLES = 10;
#else
const int BUILD_NUM_TRAIN_SAMPLES = 4000;
const int BUILD_NUM_TEST_SAMPLES = 1000;
#endif /* MDEBUG */
const int BUILD_NUM_TOTAL_SAMPLES = BUILD_NUM_TRAIN_SAMPLES + BUILD_NUM_TEST_SAMPLES;

const int BUILD_MAX_NUM_INPUTS = 10;

class BuildNetwork {
public:
	std::vector<double> inputs;

	std::vector<BuildNode*> nodes;

	std::vector<double> output_weights;
	double output_constant;
	std::vector<double> output_weight_updates;
	double output_constant_update;

	int epoch_iter;
	double average_max_update;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;
	int history_index;

	BuildNetwork(int num_inputs);
	~BuildNetwork();

	double activate(std::vector<double>& obs);
	void backprop(std::vector<double>& obs,
				  double target_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void update_helper();
	void measure_helper();
};

#endif /* BUILD_NETWORK_H */