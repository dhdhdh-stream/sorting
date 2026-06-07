// - predicted state change doesn't make sense
//   - just predicts average
//     - which can lead to no man's land
// - should generate obs or state changes instead

#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class StateNetwork;

class WorldModel {
public:
	int num_states;

	StateNetwork* obs_network;
	StateNetwork* action_network;
	StateNetwork* final_network;
	int epoch_iter;
	double average_max_update;

	double misguess_average;
	double misguess_variance_average;

	StateNetwork* predict_network;
	/**
	 * - predict roughly the impact of 1 action and 1 obs update
	 */
	int predict_epoch_iter;
	double predict_average_max_update;

	double predict_misguess_average;

	WorldModel(int num_obs,
			   int num_actions);
	WorldModel(WorldModel* original);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void add_states();

	void save(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */