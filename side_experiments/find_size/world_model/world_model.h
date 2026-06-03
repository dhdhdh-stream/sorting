#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class Network;

class WorldModel {
public:
	int num_obs;
	int num_actions;

	int num_states;

	Network* obs_network;
	Network* action_network;

	Network* score_network;

	double misguess_average;
	double misguess_variance_average;

	int epoch_iter;
	double average_max_update;

	WorldModel(int num_obs,
			   int num_actions);
	WorldModel(WorldModel* original);
	~WorldModel();

	void add_states();
	void remove_states();
};

#endif /* WORLD_MODEL_H */