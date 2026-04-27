#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class Network;

class WorldModel {
public:
	std::vector<std::vector<Network*>> location_networks;
	std::vector<Network*> state_networks;
	std::vector<Network*> predict_networks;

	/**
	 * TODO:
	 * - for simulation:
	 *   - init networks
	 *   - predict location networks
	 */

	WorldModel();
};

class WorldModelInstance {
public:
	WorldModel* world_model;

	std::vector<double> states;
	int location;

	std::vector<std::vector<double>> state_input_histories;
	std::vector<int> state_location_history;

	std::vector<std::vector<double>> predict_identity_target_histories;
	std::vector<std::vector<double>> predict_identity_input_histories;
	std::vector<int> predict_identity_location_history;

	std::vector<int> action_history;
	std::vector<std::vector<double>> location_input_histories;
	std::vector<int> rel_location_history;
	/**
	 * - simply don't backpropagate errors from location_networks
	 */

	std::vector<std::vector<double>> predict_target_histories;
	std::vector<std::vector<double>> predict_input_histories;
	std::vector<int> predict_location_history;

	WorldModelInstance(WorldModel* world_model);

	void train_init(std::vector<double>& obs);
	void train_step(int action,
					std::vector<double>& obs,
					bool add_uncertainty);
	void train_backprop();

	void debug_init(std::vector<double>& obs);
	void debug_step(int action,
					std::vector<double>& obs);
};

#endif /* WORLD_MODEL_H */