#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	std::vector<double> starting_likelihood;

	WorldModel();
	WorldModel(WorldModel* original);
	~WorldModel();

	void split_state(int state_index);
};

#endif /* WORLD_MODEL_H */